#!/usr/bin/env python3
"""Convert symbols/<m>.txt (our format: extra free-text after type:) into a strict splat symbol_addrs file
containing only `name = 0xADDR; // type:func|data`. usage: splat_symbols.py MODULE > build/MODULE.syms.txt"""
import re, sys
for line in open(f"symbols/{sys.argv[1]}.txt"):
    m = re.match(r"(\w+)\s*=\s*(0x[0-9A-Fa-f]+);\s*//\s*type:(func|data)", line)
    if m:
        # splat has no generic "data" type; emit data symbols untyped (size/type inferred from the disassembly)
        print(f"{m.group(1)} = {m.group(2)};" + ("" if m.group(3) == "data" else " // type:func"))
