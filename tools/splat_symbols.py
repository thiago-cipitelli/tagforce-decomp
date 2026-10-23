#!/usr/bin/env python3
"""Convert symbols/<m>.txt (our format: extra free-text after type:) into a strict splat symbol_addrs file
containing only `name = 0xADDR; // type:func|data`. usage: splat_symbols.py MODULE > build/MODULE.syms.txt"""
import re, sys
for line in open(f"symbols/{sys.argv[1]}.txt"):
    m = re.match(r"(\w+)\s*=\s*(0x[0-9A-Fa-f]+);\s*//\s*type:(func|data)", line)
    if m and " alias" not in line.split("type:", 1)[1][:10]:
        # splat has no generic "data" type; emit data symbols untyped (size/type inferred from the disassembly).
        # `size:0xNN` is passed through so references into the object become `sym+off` (needed when the
        # object is defined in C and the asm still references its interior).
        sz = re.search(r"\bsize:(0x[0-9A-Fa-f]+)", line)
        extra = (" // type:func" if m.group(3) == "func" else "") + (f" {'//' if m.group(3) == 'data' else ''} size:{sz.group(1)}" if sz else "")
        print(f"{m.group(1)} = {m.group(2)};" + extra)
