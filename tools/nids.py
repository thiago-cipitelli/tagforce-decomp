#!/usr/bin/env python3
"""Build tools/psp_nids.txt ("lib nid name" per line) from pspsdk's IMPORT_FUNC stub sources.
Downloads the pspsdk master tarball (~5 MB) into build/. usage: nids.py [out=tools/psp_nids.txt]"""
import io
import re
import sys
import tarfile
import urllib.request

URL = "https://github.com/pspdev/pspsdk/archive/refs/heads/master.tar.gz"
PAT = re.compile(rb'IMPORT_FUNC\s+"([A-Za-z0-9_]+)"\s*,\s*(0x[0-9A-Fa-f]{8})\s*,\s*([A-Za-z0-9_]+)')

out = sys.argv[1] if len(sys.argv) > 1 else "tools/psp_nids.txt"
data = urllib.request.urlopen(URL).read()
seen = set()
with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as tar, open(out, "w") as f:
    for m in tar:
        if m.isfile() and m.name.endswith(".S") and "/src/" in m.name:
            for lib, nid, name in PAT.findall(tar.extractfile(m).read()):
                key = (lib, nid.lower())
                if key not in seen:
                    seen.add(key)
                    f.write(f"{lib.decode()} {nid.decode()} {name.decode()}\n")
print(f"{len(seen)} NIDs -> {out}")
