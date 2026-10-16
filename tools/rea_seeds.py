#!/usr/bin/env python3
"""Write the REA Ghidra seed file (TSV) from a splat symbols file. Only lines tagged `seed` are emitted:
those are functions Ghidra's auto-analysis misses (thread entries, callbacks). usage: rea_seeds.py symbols/X.txt > build/rea_seeds.tsv"""
import re, sys
for line in open(sys.argv[1]):
    m = re.match(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//(.*)", line)
    if m and "seed" in m.group(3):
        print(f"0x{int(m.group(2),16):x}\tfunction\t{m.group(1)}")
