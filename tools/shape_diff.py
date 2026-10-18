#!/usr/bin/env python3
"""shape_diff.py SRC.c FUNC 0xADDR [--regs]: compile SRC, compare FUNC with the original ignoring register
names (structure diff). --regs keeps register names. Uses psp-objdump on raw bytes for both sides."""
import difflib, re, struct, subprocess, sys, os
sys.path.insert(0, "tools"); from mwcc_check import object_function, BASE, SEG
src, fn, addr = sys.argv[1], sys.argv[2], int(sys.argv[3], 16); keep = "--regs" in sys.argv
obj = "build/tmp/shape.o"
r = subprocess.run(["tools/mwcc.sh", "-O4,p", "-sdatathreshold", "0", "-i", "include", "-c", src, "-o", obj], capture_output=True, text=True)
if r.returncode: print(r.stdout, r.stderr); sys.exit(1)
code, relocs = object_function(obj, fn)
MOD = os.environ.get("MODULE", "BOOT")
orig = open(f"build/elf/{MOD}.elf", "rb").read()
# original size: up to the next function symbol in symbols/BOOT.txt
funcs = sorted(int(m.group(1), 16) for m in re.finditer(r"= 0x([0-9A-Fa-f]+); // type:func", open(f"symbols/{MOD}.txt").read()))
end = next((a for a in funcs if a > addr), addr + len(code))
want = orig[addr - BASE + SEG: end - BASE + SEG]
def dis(b, path):
    open(path, "wb").write(b)
    out = subprocess.run([os.path.expanduser("~/opt/pspdev/bin/psp-objdump"), "-D", "-b", "binary", "-mmips:allegrex", "-EL", path], capture_output=True, text=True).stdout
    lines = []
    for l in out.splitlines():
        m = re.match(r"\s*([0-9a-f]+):\s+[0-9a-f]{8}\s+(.*)", l)
        if not m: continue
        ins = m.group(2).strip()
        if not keep:
            ins = re.sub(r"\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|fp|gp|sp|ra|k[01])\b", "R", ins)
            ins = re.sub(r"\$f\d+", "F", ins)
            ins = re.sub(r"0x[0-9a-f]+ <[^>]*>", "TGT", ins)  # branch targets
            ins = re.sub(r"^(jal|j)\s+0x[0-9a-f]+", r"\1 TGT", ins)  # call targets (relocs differ)
        lines.append(f"{int(m.group(1),16):4x}: {ins}")
    return lines
a, b = dis(want, "build/tmp/shape_orig.bin"), dis(code, "build/tmp/shape_new.bin")
same = 0
for l in difflib.unified_diff([x.split(": ",1)[1] for x in a], [x.split(": ",1)[1] for x in b], "orig", "new", n=2, lineterm=""):
    print(l)
print(f"orig {len(a)} insns, new {len(b)} insns")
