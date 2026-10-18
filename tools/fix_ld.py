#!/usr/bin/env python3
"""Post-process a splat linker script so the data layout matches the Metrowerks link:
 1. inside the DATA block, order input sections by their original file offset (from the yaml),
 2. the C objects' .rodata is merged right after their .data (the original modules have no .rodata).
usage: fix_ld.py config/MODULE.yaml build/MODULE.ld"""
import re, sys
yaml, ld = sys.argv[1], sys.argv[2]
order = {}  # object basename -> offset
for line in open(yaml):
    m = re.match(r"\s*-\s*\[0x([0-9A-Fa-f]+),\s*(\S+),\s*(\w+)\]", line)
    if m:
        order[m.group(3)] = int(m.group(1), 16)
text = open(ld).read()

# --- TEXT block: a C file split by `@asm` ranges appears several times as `obj(.text)`; replace each
# occurrence by that range's functions as `obj(.text.<func>)` (sections renamed by tools/rename_sections.py)
import glob as _glob, os as _os
module = _os.path.basename(yaml)[:-5]
subs = []  # (offset, type, name) of the code segment's subsegments
seg_off = seg_vram = None
for line in open(yaml):
    m = re.match(r"\s*start:\s*0x([0-9A-Fa-f]+)", line)
    if m and seg_off is None: seg_off = int(m.group(1), 16)
    m = re.match(r"\s*vram:\s*0x([0-9A-Fa-f]+)", line)
    if m and seg_vram is None: seg_vram = int(m.group(1), 16)
    m = re.match(r"\s*-\s*\[0x([0-9A-Fa-f]+),\s*(\w+),\s*(\w+)\]", line)
    if m: subs.append((int(m.group(1), 16), m.group(2), m.group(3)))
subs.sort()
funcs = {}
for line in open(f"symbols/{module}.txt"):
    m = re.match(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*type:func", line)
    if m: funcs[int(m.group(2), 16)] = m.group(1)
c_count = {}
for k, (off, typ, nm) in enumerate(subs):
    if typ != "c": continue
    c_count[nm] = c_count.get(nm, 0) + 1
if any(n > 1 for n in c_count.values()):
    for k, (off, typ, nm) in enumerate(subs):
        if typ != "c" or c_count[nm] < 2: continue
        lo = seg_vram + off - seg_off
        hi = seg_vram + (subs[k + 1][0] - seg_off) if k + 1 < len(subs) else 1 << 32
        names = [funcs[a] for a in sorted(funcs) if lo <= a < hi]
        pat = re.compile(rf"^(\s*)(\S*/{re.escape(nm)}\.c\.o)\(\.text\);?\s*$", re.M)
        mm = pat.search(text)
        if not mm: raise SystemExit(f"fix_ld: {nm}.c.o(.text) not found for subsegment {k}")
        repl = "\n".join(f"{mm.group(1)}{mm.group(2)}(.text.{f});" for f in names)
        text = text[:mm.start()] + repl + text[mm.end():]
    print(f"fix_ld: expanded per-function sections for {[n for n, c in c_count.items() if c > 1]}")
# every other C object: take all its renamed function sections at once
text = re.sub(r"(\S+\.c\.o)\(\.text\);", r"\1(.text .text.*);", text)
def block(name):
    s = text.index(f"{name}_START = .;"); e = text.index(f"{name}_END = .;")
    return s, e
mod = re.search(r"(\w+)_TEXT_START = \.;", text).group(1)  # the module's own blocks, not header_DATA
ds, de = block(mod + "_DATA")
data_lines = [l for l in text[ds:de].splitlines()[1:] if l.strip()]
rs, re_ = block(mod + "_RODATA")
ro_lines = [l for l in text[rs:re_].splitlines()[1:] if l.strip()]
def key(l):
    m = re.search(r"/([\w.]+)\.(c|s|bin)\.o\(", l)
    nm = m.group(1).split("/")[-1]
    nm = nm.replace(".data", "")
    return order.get(nm, 10**9)
merged = []
has_rodata_range = {m.group(1) for m in re.finditer(r"/(\w+)__rodata\.c\.o\(\.data\)", text)}
for l in sorted(data_lines, key=key):
    rm = re.search(r"/(\w+)__rodata\.c\.o\(\.data\)", l)
    if rm:  # @rodata placeholder -> the real object's .rodata at this position
        merged.append(l.replace(f"{rm.group(1)}__rodata.c.o(.data)", f"{rm.group(1)}.c.o(.rodata)"))
        ro_lines = [r for r in ro_lines if f"/{rm.group(1)}.c.o(.rodata)" not in r]
        continue
    merged.append(l)
    m = re.search(r"/([\w.]+)\.c\.o\(\.data\)", l)
    if m and m.group(1) in has_rodata_range:
        continue
    if m:
        ro = [r for r in ro_lines if f"/{m.group(1)}.c.o(.rodata)" in r]
        merged += ro
        ro_lines = [r for r in ro_lines if r not in ro]
new = text[:ds] + text[ds:de].splitlines()[0] + "\n" + "\n".join(merged) + "\n        " + text[de:rs] + text[rs:re_].splitlines()[0] + "\n" + "\n".join(ro_lines) + ("\n        " if ro_lines else "        ") + text[re_:]
# static C functions still called from asm (an asm-kept function of the same file): the asm needs a
# global name, so define it at the function's address
provides = []
for line in open(f"symbols/{module}.txt"):
    m = re.match(r"(\w+)\s*=\s*(0x[0-9A-Fa-f]+);\s*//\s*type:func static", line)
    if m:
        provides.append(f"PROVIDE({m.group(1)} = {m.group(2)});")
if provides:
    new = new.rstrip("\n") + "\n" + "\n".join(provides) + "\n"
open(ld, "w").write(new)
print(f"fix_ld: {len(merged)} data entries ordered, {len(ro_lines)} rodata entries left")
