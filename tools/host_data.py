#!/usr/bin/env python3
"""Emit a module's .data/.bss as x86 GNU as source for the host (PC) link: build/host/data/<m>.s.

In the matching build the data comes from splat's asm, never from C, so the host link needs it too. The bytes are
copied from build/elf/<m>.elf in address order (offsets stay those of the PSP). Every R_MIPS_32 relocation that lands
in the data (read from the PRX relocation sections that prx2elf leaves in the ELF) becomes `.long <target>`:
  - function start in .text, or import stub        -> its name from symbols/<m>.txt (0 if `funcs` lacks it)
  - .text but not a function start (jump table)      -> 0 (the host C has its own switch tables)
  - .data/.bss inside an object the host C defines   -> csym+off
  - other .data/.bss                                 -> a local label at the target, in this copy
  - anything else (module info, .lib.ent ...)        -> 0
Objects the host C defines (address from symbols/ or, for the rest, from the matching link build/<m>.rebuilt.elf) are
not copied: their bytes and the relocations inside them are dropped, pointers to them become csym+off.
Global labels: every data symbol of symbols/<m>.txt (and every needed D_XXXXXXXX) that the host C does not define.
A name that falls inside an object the host C defines (an alias of a table element) is not labelled here: it goes to
`defsyms` (name -> csym+off) for the module's `ld -r --defsym`.

usage: host_data.py MODULE [OUT.s]      (no host objects: labels every data symbol)
       host_data.py --selftest MODULE   assemble and compare every non-relocated byte with the ELF
"""
import glob
import os
import re
import struct
import subprocess
import sys
import tempfile

BASE = 0x08804000
R_MIPS_32 = 2
SYM_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(0x[0-9A-Fa-f]+);\s*//\s*type:(\w+)(.*)$")
DNAME_RE = re.compile(r"^D_([0-9A-Fa-f]{8})$")
FNAME_RE = re.compile(r"^func_([0-9A-Fa-f]{8})$")
# `extern T name...; /* 0x08XXXXXX ...` in the module's C: globals the C names that symbols/ does not have yet
EXTERN_RE = re.compile(r"extern\b[^;(]*?\b(\w+)\s*(?:\[[^\]]*\]\s*)*;\s*/\*\s*(0x08[0-9A-Fa-f]{6})\b")
EXTERN_FP_RE = re.compile(r"extern\b[^;]*?\(\s*\*\s*(\w+)\s*\)\s*\([^;]*\)\s*;\s*/\*\s*(0x08[0-9A-Fa-f]{6})\b")
EXTRA = {"__fdlib_version": 0x08873880}  # src/BOOT/libm.c: address in the comment above the declaration


def comment_addrs(module):
    out = dict(EXTRA)
    for path in glob.glob(f"src/{module}/*.c"):
        text = open(path, encoding="utf-8", errors="replace").read()
        for m in list(EXTERN_RE.finditer(text)) + list(EXTERN_FP_RE.finditer(text)):
            out.setdefault(m.group(1), int(m.group(2), 16))
    return out


def read_elf(path):
    img = open(path, "rb").read()
    shoff = struct.unpack_from("<I", img, 0x20)[0]
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", img, 0x2E)
    secs = [struct.unpack_from("<IIIIIIIIII", img, shoff + i * shentsize) for i in range(shnum)]
    stroff = secs[shstrndx][4]

    def name(s):
        return img[stroff + s[0]:img.index(b"\0", stroff + s[0])].decode()
    return img, [(name(s),) + s for s in secs]


def load_symbols(module):
    """addr -> [(name, type, is_import)] in file order."""
    out = {}
    for line in open(f"symbols/{module}.txt"):
        m = SYM_RE.match(line)
        if m:
            out.setdefault(int(m.group(2), 16), []).append((m.group(1), m.group(3), "import:" in m.group(4)))
    return out


def module_layout(module):
    img, secs = read_elf(f"build/elf/{module}.elf")
    data = []   # (name, addr, size, bytes or None for nobits, align)
    text = None
    other = []  # alloc sections we do not emit
    for nm, _n, typ, flags, addr, off, size, *_r, align, _es in secs:
        if not flags & 2 or nm == ".MIPS.abiflags":
            continue
        if flags & 1:  # writable: .data / .bss
            data.append((nm, addr, size, None if typ == 8 else img[off:off + size], align))
        elif nm == ".text":
            text = (addr, addr + size)
        else:
            other.append((nm, addr, addr + size))
    # PRX relocation sections are now SHT_NULL but keep their bytes; r_offset is relative to the segment (vaddr 0)
    rels = {}
    for nm, _n, typ, flags, addr, off, size, *_r in secs:
        if nm.startswith(".rel") and typ == 0:
            for j in range(off, off + size, 8):
                r_off, r_info = struct.unpack_from("<II", img, j)
                a = r_off + BASE
                if any(s[1] <= a < s[1] + s[2] for s in data if s[3] is not None):
                    if r_info & 0xFF != R_MIPS_32:
                        raise SystemExit(f"{module}: relocation type {r_info & 0xFF} in data at {a:#x}")
                    rels[a] = struct.unpack_from("<I", data_bytes(data, a), 0)[0]
    return data, text, other, rels


def data_bytes(data, a):
    for _nm, addr, size, b, _al in data:
        if b is not None and addr <= a < addr + size:
            return b[a - addr:a - addr + 4]
    raise KeyError(a)


def rebuilt_addrs(module):
    """name -> (addr, size) of the data objects of build/<m>.rebuilt.elf (the matching link: where mwcc put the
    globals and file statics the C defines; names unique in the module only). Empty when it was not built."""
    path = f"build/{module}.rebuilt.elf"
    if not os.path.exists(path):
        return {}
    img, secs = read_elf(path)
    out, dup = {}, set()
    for s in secs:
        if s[0] != ".symtab":
            continue
        strtab = secs[s[7]][5]
        for off in range(s[5], s[5] + s[6], 16):
            st_name, value, size, info = struct.unpack_from("<IIIB", img, off)
            if info & 0xF != 1 or not st_name:  # STT_OBJECT
                continue
            n = img[strtab + st_name:img.index(b"\0", strtab + st_name)].decode()
            if n in out:
                dup.add(n)
            out[n] = (value, size)
    for n in dup:
        del out[n]
    return out


def generate(module, cdefs=None, funcs=None, need=()):
    """cdefs: name -> host size of the globals the module's host objects define (None: no host objects).
    funcs: function names the host link can reach (the module's objects, a static only when unique, platform/pc);
    None = all. Import stubs always resolve by name.
    need: names the objects leave undefined.
    Returns (asm text, defsyms {name: 'csym+off'}, stats)."""
    cdefs = cdefs or {}
    syms = load_symbols(module)
    data, text, other, rels = module_layout(module)
    lo = min(s[1] for s in data)
    hi = max(s[1] + s[2] for s in data)

    # data symbols of this module, by address (first name per address is the "object" name)
    dsyms = sorted((a, n) for a, l in syms.items() if lo <= a < hi for n, t, imp in l if not imp)
    for n in need:
        m = DNAME_RE.match(n)
        if m and lo <= int(m.group(1), 16) < hi:
            dsyms.append((int(m.group(1), 16), n))
    known = {n for l in syms.values() for n, _t, _i in l}
    for n, a in comment_addrs(module).items():
        if n in need and n not in known and lo <= a < hi:
            dsyms.append((a, n))
    dsyms.sort()
    # objects the host C defines: address from symbols/ (host size), else from the matching link (PSP size)
    caddr = {n: (a, cdefs[n]) for a, n in dsyms if n in cdefs}
    if cdefs:
        for n, (a, sz) in rebuilt_addrs(module).items():
            if n in cdefs and n not in caddr and lo <= a < hi:
                caddr[n] = (a, sz)
    cobjs = sorted((a, n, sz) for n, (a, sz) in caddr.items())

    def in_cobj(a):
        """(csym, off) when address a lies inside an object the host C defines."""
        best = None
        for ca, cn, sz in cobjs:
            if ca > a:
                break
            if a < ca + max(sz, 1):
                best = (cn, a - ca)
        return best

    labels = {}   # addr -> [global names]
    defsyms = {}
    seen = set()
    for a, n in dsyms:
        if n in seen or n in cdefs:
            continue
        seen.add(n)
        c = in_cobj(a)
        if c:
            defsyms[n] = f"{c[0]}+{c[1]}" if c[1] else c[0]
        else:
            labels.setdefault(a, []).append(n)

    # func_XXXXXXXX: a C file that calls a function by its address, named otherwise in symbols/
    for n in need:
        m = FNAME_RE.match(n)
        fn = m and next((x for x, t, imp in syms.get(int(m.group(1), 16), []) if t == "func" and x != n), None)
        if fn and (funcs is None or fn in funcs):
            defsyms[n] = fn

    # bytes of the objects the host C defines are dropped (every pointer to them is csym+off now), with the
    # relocations inside them
    cranges = [(a, a + max(sz, 1)) for a, _n, sz in cobjs]

    def in_c(p):
        return any(a <= p < b for a, b in cranges)
    rels = {a: t for a, t in rels.items() if not in_c(a)}
    stats = {"dropped": sum(b - a for a, b in cranges), "rel": len(rels), "rel_zero": 0, "rel_local": 0, "rel_func": 0, "rel_c": 0,
             "labels": sum(len(v) for v in labels.values()), "defsyms": len(defsyms)}
    locs = set()
    relexpr = {}
    for a, t in sorted(rels.items()):
        names = [n for n, ty, imp in syms.get(t, []) if ty == "func"]
        if text[0] <= t < text[1] or any(s[1] <= t < s[2] for s in other):
            fn = next((n for n, ty, imp in syms.get(t, []) if ty == "func" and (imp or funcs is None or n in funcs)),
                      None)
            if fn:
                relexpr[a] = fn
                stats["rel_func"] += 1
            else:
                why = "no host function " + names[0] if names else "not a function start"
                relexpr[a] = f"0 /* {t:#010x}: {why} */"
                stats["rel_zero"] += 1
        elif lo <= t <= hi:
            c = in_cobj(t)
            if c:
                relexpr[a] = f"{c[0]}+{c[1]}" if c[1] else c[0]
                stats["rel_c"] += 1
            else:
                relexpr[a] = f".L{t:08X}"
                locs.add(t)
                stats["rel_local"] += 1
        else:
            relexpr[a] = f"0 /* {t:#010x}: outside the module */"
            stats["rel_zero"] += 1

    out = [f"# generated by tools/host_data.py from build/elf/{module}.elf: do not edit", "\t.att_syntax"]
    for k, (nm, addr, size, b, align) in enumerate(data):
        out.append("\t.bss" if b is None else "\t.data")
        # keep the PSP address phase (mod 4096): the game checks alignments the ELF layout gave for free
        # (snd_sas_core refuses a mix buffer that is not 64-byte aligned, port step 6)
        out.append("\t.balign 4096")
        out.append(f"\t.skip {addr & 0xFFF}")
        end = addr + size
        # labels in the gap after this section (or at the very end) are placed at its end
        own_end = data[k + 1][1] if k + 1 < len(data) else hi + 1
        stops = sorted({p for p in list(labels) + list(locs) if addr <= p < own_end} |
                       {a for a in relexpr if addr <= a < end and b is not None and not in_c(a)} |
                       {p for r in cranges for p in r if addr <= p < end} | {end})
        pos = addr
        for p in stops:
            q = min(p, end)
            if q > pos and in_c(pos):   # the C defines this object: filler keeps the copy's layout (and phase)
                out.append(f"\t.{'zero' if b is None else 'skip'} {q - pos}")
                pos = q
            if q > pos:
                if b is None:
                    out.append(f"\t.zero {q - pos}")
                else:
                    chunk = b[pos - addr:q - addr]
                    for i in range(0, len(chunk), 16):
                        out.append("\t.byte " + ",".join(str(x) for x in chunk[i:i + 16]))
                pos = q
            if p >= own_end:
                continue
            for n in labels.get(p, []):
                out += [f"\t.globl {n}", f"{n}:"]
            if p in locs:
                out.append(f".L{p:08X}:")
            if p in relexpr and b is not None and p < end and not in_c(p):
                if pos != p:
                    raise SystemExit(f"{module}: overlapping relocation at {p:#x}")
                out.append(f"\t.long {relexpr[p]}")
                pos = p + 4
        if pos < end:
            if b is None:
                out.append(f"\t.zero {end - pos}")
            else:
                chunk = b[pos - addr:]
                for i in range(0, len(chunk), 16):
                    out.append("\t.byte " + ",".join(str(x) for x in chunk[i:i + 16]))
    return "\n".join(out) + "\n", defsyms, stats


def obj_sections(path):
    """name -> (bytes or None, size) for an ELF32 relocatable."""
    img, secs = read_elf(path)
    return {s[0]: (None if s[2] == 8 else img[s[5]:s[5] + s[6]], s[6]) for s in secs}


def selftest(module):
    asm, _d, st = generate(module)
    data, _t, _o, rels = module_layout(module)
    with tempfile.TemporaryDirectory() as tmp:
        s, o = os.path.join(tmp, "d.s"), os.path.join(tmp, "d.o")
        open(s, "w").write(asm)
        subprocess.run(["as", "--32", s, "-o", o], check=True)
        got = obj_sections(o)
    dpos = 0
    bad = 0
    for nm, addr, size, b, _al in data:
        key = ".bss" if b is None else ".data"
        gb, gsize = got[key]
        if b is None:
            assert gsize >= size, (nm, gsize, size)
            continue
        dpos = (dpos + 0xFFF & ~0xFFF) + (addr & 0xFFF)   # .balign 4096 + .skip phase
        gb = gb[dpos:]
        for i in range(0, size, 4):
            if addr + i in rels:
                continue
            if b[i:i + 4] != gb[i:i + 4]:
                bad += 1
        dpos += size
    print(f"{module}: {st}  mismatched words: {bad}")
    assert bad == 0
    return 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if not a:
        raise SystemExit(__doc__)
    if a[0] == "--selftest":
        sys.exit(selftest(a[1] if len(a) > 1 else "libcharalist"))
    text, _d, st = generate(a[0])
    if len(a) > 1:
        open(a[1], "w").write(text)
    print(st, file=sys.stderr)
