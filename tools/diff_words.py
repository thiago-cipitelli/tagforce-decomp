#!/usr/bin/env python3
"""List differing 32-bit words of one section between two ELFs (by vram). usage: diff_words.py ORIG NEW .text [max]"""
import struct, sys
sys.path.insert(0, "tools"); from compare_sections import sections
o, n = sections(sys.argv[1]), sections(sys.argv[2])
vram, od = o[sys.argv[3]]
nd = next(d[vram - v:vram - v + len(od)] for v, d in n.values() if v <= vram < v + len(d))
k = 0
for i in range(0, len(od), 4):
    a, b = od[i:i+4], nd[i:i+4]
    if a != b:
        print(f"{vram+i:#010x}  orig {struct.unpack('<I',a)[0]:08x}  new {struct.unpack('<I',b)[0]:08x}"); k += 1
        if k >= int(sys.argv[4] if len(sys.argv) > 4 else 40): break
print(f"{sum(od[i:i+4]!=nd[i:i+4] for i in range(0,len(od),4))} words differ")
