#!/usr/bin/env python3
"""find_twins.py [--min N] [--ratio R] [--all] [--check]: twin functions across all modules.

Reads every build/elf/<m>.elf and symbols/<m>.txt (function size = distance to the next type:func symbol, or to
the end of .text), masks what relocation changes (j/jal targets, lui immediates, the immediate of an I-type whose
base register came from a lui) and groups functions whose masked word sequences are identical ("=") or, for
queue functions (docs/matching_queue.tsv), near-identical ("~", difflib ratio >= R, default 0.90) to a matched one.
Default output: only groups with at least one matched function and one queue function in ANOTHER module (the
ones worth porting a shape from). --all prints every group with a queue function. --min: minimum size in bytes
(default 64). Sanity check: --check (dk_draw_info/dt_draw_info/sc_draw_info must be exact twins)."""
import difflib, glob, os, re, struct, sys
from collections import defaultdict

BASE, SEG = 0x08804000, 0x60
SYM = re.compile(r"^(\w+) = 0x([0-9A-Fa-f]+); // type:func(.*)")


def text_range(elf):
    shoff, = struct.unpack_from("<I", elf, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", elf, 0x2E)
    sh = [struct.unpack_from("<IIIIII", elf, shoff + i * shentsize) for i in range(shnum)]
    strtab = sh[shstrndx][4]
    for name, _, _, addr, _, size in sh:
        if elf[strtab + name:elf.index(b"\0", strtab + name)] == b".text":
            return addr, addr + size
    raise SystemExit("no .text")


def mask(words):
    out, lui = [], set()
    for w in words:
        op, rs, rt = w >> 26, (w >> 21) & 31, (w >> 16) & 31
        if op in (2, 3):                       # j/jal
            w &= 0xFC000000
        elif op == 0x0F:                       # lui
            w &= 0xFFFF0000; lui.add(rt); out.append(w); continue
        elif op >= 8 and rs in lui:            # %lo from a lui base
            w &= 0xFFFF0000
        if op >= 8 and op not in (0x2B, 0x29, 0x28, 0x39, 0x3D):  # I-type write to rt (not stores)
            lui.discard(rt)
        elif op == 0:
            lui.discard((w >> 11) & 31)
        out.append(w)
    return tuple(out)


def load():
    queue = {}
    for l in open("docs/matching_queue.tsv").read().splitlines()[1:]:
        f = l.split("\t")
        queue[(f[2], f[3])] = 1
    funcs = []                                 # (module, name, addr, size, status, masked)
    for path in sorted(glob.glob("symbols/*.txt")):
        m = os.path.basename(path)[:-4]
        if not os.path.exists(f"build/elf/{m}.elf"):
            continue
        elf = open(f"build/elf/{m}.elf", "rb").read()
        lo, hi = text_range(elf)
        syms = {}
        for l in open(path):
            s = SYM.match(l)
            if s and lo <= int(s.group(2), 16) < hi:
                syms.setdefault(int(s.group(2), 16), (s.group(1), "matched" in s.group(3)))
        addrs = sorted(syms) + [hi]
        for a, b in zip(addrs, addrs[1:]):
            name, matched = syms[a]
            st = "matched" if matched else "queue" if (m, name) in queue else "other"
            ws = struct.unpack_from(f"<{(b - a) // 4}I", elf, a - BASE + SEG)
            funcs.append((m, name, a, b - a, st, mask(ws)))
    return funcs


def groups(funcs, minsize, ratio):
    exact = defaultdict(list)
    for f in funcs:
        if f[3] >= minsize:
            exact[f[5]].append(f)
    out = [("=", g) for g in exact.values() if len(g) > 1]
    seen = {id(f) for _, g in out for f in g if any(x[4] == "matched" for x in g)}
    matched = [f for f in funcs if f[4] == "matched" and f[3] >= minsize]
    for q in funcs:                            # near twins: queue function vs matched ones of similar size
        if q[4] != "queue" or q[3] < minsize or id(q) in seen:
            continue
        sm = difflib.SequenceMatcher(None, autojunk=False)
        sm.set_seq2(q[5])
        near = []
        for f in matched:
            if f[0] == q[0] or abs(f[3] - q[3]) > q[3] * (1 - ratio):
                continue
            sm.set_seq1(f[5])
            if sm.real_quick_ratio() >= ratio and sm.quick_ratio() >= ratio and sm.ratio() >= ratio:
                near.append((round(sm.ratio(), 3), f))
        if near:
            near.sort(key=lambda x: -x[0])
            out.append(("~", [q] + [f for _, f in near[:4]]))
    return out


def fmt(kind, g):
    return f"{kind} " + "  ".join(f"{m}:{n}({s},{st})" for m, n, _, s, st, _ in g)


def main(argv):
    if "--check" in argv:
        f = {(x[0], x[1]): x for x in load()}
        a, b, c = (f[k] for k in [("libdeck", "dk_draw_info"), ("libdeck_tutorial", "dt_draw_info"),
                                  ("libselect_card", "sc_draw_info")])
        assert a[5] == b[5] == c[5] and a[3] == 988, "draw_info twins not identical"
        assert mask([0x0C200000 | 0x1234]) == mask([0x0C200000 | 0x4321])
        print("find_twins.py: ok"); return
    minsize = int(argv[argv.index("--min") + 1]) if "--min" in argv else 64
    ratio = float(argv[argv.index("--ratio") + 1]) if "--ratio" in argv else 0.90
    for kind, g in groups(load(), minsize, ratio):
        has_q = [f for f in g if f[4] == "queue"]
        if not has_q:
            continue
        if "--all" not in argv and not any(f[4] == "matched" and any(q[0] != f[0] for q in has_q) for f in g):
            continue
        print(fmt(kind, g))


if __name__ == "__main__":
    main(sys.argv[1:])
