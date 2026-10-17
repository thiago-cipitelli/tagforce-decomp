#!/usr/bin/env python3
"""Compile one reconstructed C file and compare every function of its `// @range` against the original
module bytes (relocated fields masked, see mwcc_check.py). Function names come from symbols/<m>.txt.
usage: check_file.py MODULE src/MODULE/file.c [--flags "..."]   (exit 0 when every function matches)"""
import os, re, struct, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mwcc_check import object_function, BASE, SEG, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16

module, src = sys.argv[1], sys.argv[2]
flags = sys.argv[sys.argv.index("--flags") + 1] if "--flags" in sys.argv else "-O4,p -sdatathreshold 0 -i include"
text = open(src).read()
lo, hi = (int(x, 16) for x in re.search(r"//\s*@range\s+0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)", text).groups())
funcs = sorted((int(m.group(2), 16), m.group(1)) for m in re.finditer(r"^(\w+)\s*=\s*(0x[0-9A-Fa-f]+);\s*//\s*type:func", open(f"symbols/{module}.txt").read(), re.M)
         if lo <= int(m.group(2), 16) < hi)
obj = f"build/{os.path.basename(src)[:-2]}.check.o"
r = subprocess.run(["tools/mwcc.sh", *flags.split(), "-c", src, "-o", obj], capture_output=True, text=True)
if r.returncode:
    print(r.stdout, r.stderr); sys.exit("compile failed")
orig = open(f"build/elf/{module}.elf", "rb").read()
bad = 0
for addr, name in funcs:
    try:
        code, relocs = object_function(obj, name)
    except SystemExit:
        print(f"??  {name:28s} not in object"); bad += 1; continue
    want = orig[addr - BASE + SEG: addr - BASE + SEG + len(code)]
    d = 0
    for k in range(0, len(code), 4):
        w, v = struct.unpack_from("<I", code, k)[0], struct.unpack_from("<I", want, k)[0]
        t = relocs.get(k)
        ok = (w >> 26) == (v >> 26) if t == R_MIPS_26 else (w >> 16) == (v >> 16) if t in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16) else w == v
        d += not ok
    bad += d > 0
    print(f"{'OK ' if not d else 'BAD'} {name:28s} {addr:#010x} {len(code):5d} B" + ("" if not d else f"  {d} words differ"))
print(f"{len(funcs) - bad}/{len(funcs)} functions match")
sys.exit(1 if bad else 0)
