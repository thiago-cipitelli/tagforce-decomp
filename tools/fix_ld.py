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
def block(name):
    s = text.index(f"{name}_START = .;"); e = text.index(f"{name}_END = .;")
    return s, e
ds, de = block("libhtml_view_DATA") if "libhtml_view_DATA_START" in text else block(re.search(r"(\w+)_DATA_START = \.;", text).group(1) + "_DATA")
data_lines = [l for l in text[ds:de].splitlines()[1:] if l.strip()]
rs, re_ = block(re.search(r"(\w+)_RODATA_START = \.;", text).group(1) + "_RODATA")
ro_lines = [l for l in text[rs:re_].splitlines()[1:] if l.strip()]
def key(l):
    m = re.search(r"/([\w.]+)\.(c|s|bin)\.o\(", l)
    nm = m.group(1).split("/")[-1]
    nm = nm.replace(".data", "")
    return order.get(nm, 10**9)
merged = []
for l in sorted(data_lines, key=key):
    merged.append(l)
    m = re.search(r"/([\w.]+)\.c\.o\(\.data\)", l)
    if m:
        ro = [r for r in ro_lines if f"/{m.group(1)}.c.o(.rodata)" in r]
        merged += ro
        ro_lines = [r for r in ro_lines if r not in ro]
new = text[:ds] + text[ds:de].splitlines()[0] + "\n" + "\n".join(merged) + "\n        " + text[de:rs] + text[rs:re_].splitlines()[0] + "\n" + "\n".join(ro_lines) + ("\n        " if ro_lines else "        ") + text[re_:]
open(ld, "w").write(new)
print(f"fix_ld: {len(merged)} data entries ordered, {len(ro_lines)} rodata entries left")
