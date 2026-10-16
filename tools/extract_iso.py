#!/usr/bin/env python3
"""Extract the game ISO into a directory. usage: extract_iso.py GAME.iso OUT_DIR  (needs: pip install pycdlib)"""
import os
import sys

import pycdlib

iso = pycdlib.PyCdlib()
iso.open(sys.argv[1])
out = sys.argv[2]
n = 0
for dirname, _, files in iso.walk(iso_path="/"):
    os.makedirs(out + dirname, exist_ok=True)
    for f in files:
        p = dirname.rstrip("/") + "/" + f
        iso.get_file_from_iso(out + p.split(";")[0], iso_path=p)
        n += 1
print(f"{n} files -> {out}")
