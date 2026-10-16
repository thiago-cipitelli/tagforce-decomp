#!/usr/bin/env python3
"""Resolve gmodule import stubs to BOOT (libehsys) functions by NID and emit splat symbols for each gmodule.

BOOT exports library `libehsys`; every gmodule imports it by NID. Each stub in a gmodule's .sceStub.text
gets the BOOT function's name from symbols/BOOT.txt when known, else `ehsys_<BOOT addr>`.
Also handles gmodule-to-gmodule imports (e.g. libduel_draw -> libduel_eng) and keeps sce* imports named
from the pspsdk NID table if given.

usage: link_map.py build/elf symbols [nids.txt]
writes symbols/<module>.txt (imports section only; hand-written lines are preserved above a marker).
"""
import glob
import os
import re
import struct
import sys

BASE = 0x08804000
MARK = "// --- generated imports below (tools/link_map.py); edit lines above only ---"


def u32(d, o):
    return struct.unpack_from("<I", d, o)[0]


def load(path):
    d = open(path, "rb").read()
    seg = u32(d, u32(d, 0x1C) + 4)
    shoff, shnum, shstr = u32(d, 0x20), struct.unpack_from("<H", d, 0x30)[0], struct.unpack_from("<H", d, 0x32)[0]
    secs = [struct.unpack_from("<IIIIIIIIII", d, shoff + i * 40) for i in range(shnum)]
    st = secs[shstr][4]
    by = {d[st + s[0]:d.index(b"\0", st + s[0])].decode(): s for s in secs}
    cstr = lambda v: d[v - BASE + seg:d.index(b"\0", v - BASE + seg)].decode(errors="replace")
    v2o = lambda v: v - BASE + seg
    return d, by, cstr, v2o


def exports(path):
    d, by, cstr, v2o = load(path)
    out = {}
    ent = by[".lib.ent"]
    e = ent[4]
    while e < ent[4] + ent[5]:
        nm, ver, attr, ln, vc, fc, tb = struct.unpack_from("<IHHBBHI", d, e)
        lib = cstr(nm) if nm else None
        if lib:
            for k in range(fc):
                out[(lib, u32(d, v2o(tb) + 4 * k))] = u32(d, v2o(tb) + 4 * (vc + fc + k))
        e += ln * 4
    return out


def imports(path):
    d, by, cstr, v2o = load(path)
    out = []
    stub = by[".lib.stub"]
    e = stub[4]
    while e < stub[4] + stub[5]:
        nm, ver, attr, ln, vc, fc, nt, stt = struct.unpack_from("<IHHBBHII", d, e)
        lib = cstr(nm)
        for k in range(fc):
            out.append((lib, u32(d, v2o(nt) + 4 * k), stt + 8 * k))
        e += ln * 4
    return out


def read_symbols(path):
    names = {}
    if os.path.exists(path):
        for line in open(path):
            m = re.match(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);", line)
            if m:
                names[int(m.group(2), 16)] = m.group(1)
    return names


def main(elf_dir, sym_dir, nid_file=None):
    sce = {}
    if nid_file:
        for line in open(nid_file):
            lib, nid, name = line.split()
            sce.setdefault(int(nid, 16), name)
    mods = {os.path.basename(p)[:-4]: p for p in glob.glob(os.path.join(elf_dir, "*.elf"))}
    exp = {}
    for m, p in mods.items():
        exp.update(exports(p))
    names = {m: read_symbols(os.path.join(sym_dir, m + ".txt")) for m in mods}
    for m, p in mods.items():
        if m == "BOOT":
            continue
        lines = []
        for lib, nid, stub in imports(p):
            if (lib, nid) in exp:
                src = "BOOT" if lib == "libehsys" else lib
                target = exp[(lib, nid)]
                name = names.get(src, {}).get(target) or f"{'ehsys' if lib == 'libehsys' else lib}_{target:08X}"
            else:
                name = sce.get(nid) or f"{lib}_{nid:08X}"
            lines.append(f"{name} = 0x{stub:08X}; // type:func import:{lib} nid:{nid:08X}")
        out = os.path.join(sym_dir, m + ".txt")
        head = ""
        if os.path.exists(out):
            head = open(out).read().split(MARK)[0]
        with open(out, "w") as f:
            f.write(head + MARK + "\n" + "\n".join(lines) + "\n")
        print(f"{m}: {len(lines)} imports")


if __name__ == "__main__":
    main(*sys.argv[1:])
