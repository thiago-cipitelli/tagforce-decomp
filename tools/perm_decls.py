#!/usr/bin/env python3
"""Brute-force source variants against the original bytes. Permutes local declaration order: TEMPLATE has {NAME} and {DECLS}; DECLS is a list of declaration strings.
Emits one function per permutation into a TU, compiles, compares each with the original bytes (relocs masked).
usage: perm_decls.py TEMPLATE_FILE ADDR decl1 decl2 ...  or  --alts alt1 alt2 ...
  (TEMPLATE_FILE: C with {NAME}/{DECLS}; header lines before '//--' are kept once)"""
import itertools, os, struct, subprocess, sys
sys.path.insert(0, "tools"); from mwcc_check import object_function, BASE, SEG, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16
tpl = open(sys.argv[1]).read(); addr = int(sys.argv[2], 16); decls = sys.argv[3:]
head, body = tpl.split("//--\n")
if decls and decls[0] == "--alts":  # each remaining arg is a complete alternative for {DECLS}
    perms = [(d,) for d in decls[1:]]
else:
    perms = list(itertools.permutations(decls))
src = head + "\n".join(body.replace("{NAME}", f"f_{i}").replace("{DECLS}", "\n    ".join(p)) for i, p in enumerate(perms))
open("build/tmp/perm.c", "w").write(src)
r = subprocess.run(["tools/mwcc.sh", "-O4,p", "-sdatathreshold", "0", "-i", "include", "-c", "build/tmp/perm.c", "-o", "build/tmp/perm.o"], capture_output=True, text=True)
if r.returncode: print(r.stdout, r.stderr); sys.exit(1)
orig = open("build/elf/BOOT.elf", "rb").read()
best = []
for i, p in enumerate(perms):
    code, relocs = object_function("build/tmp/perm.o", f"f_{i}")
    want = orig[addr - BASE + SEG: addr - BASE + SEG + len(code)]
    d = 0
    for k in range(0, len(code), 4):
        w, v = struct.unpack_from("<I", code, k)[0], struct.unpack_from("<I", want, k)[0]
        t = relocs.get(k)
        ok = (w >> 26) == (v >> 26) if t == R_MIPS_26 else (w >> 16) == (v >> 16) if t in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16) else w == v
        d += not ok
    best.append((d, i, p))
best.sort()
for d, i, p in best[:5]:
    print(d, "diffs:", " | ".join(p))
d, i, p = best[0]
code, relocs = object_function("build/tmp/perm.o", f"f_{i}")
want = orig[addr - BASE + SEG: addr - BASE + SEG + len(code)]
for k in range(0, len(code), 4):
    w, v = struct.unpack_from("<I", code, k)[0], struct.unpack_from("<I", want, k)[0]
    t = relocs.get(k)
    ok = (w >> 26) == (v >> 26) if t == R_MIPS_26 else (w >> 16) == (v >> 16) if t in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16) else w == v
    if not ok: print(f"  +{k:#x}: orig {v:08x} new {w:08x}")
