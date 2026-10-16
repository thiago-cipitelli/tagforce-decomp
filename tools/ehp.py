#!/usr/bin/env python3
"""EHP archive tool (format recovered from BOOT.BIN ehp_find 0x08833bc8 / ehp_entry_size 0x08833f48).

Layout:  "EHP" ver(u8)  total_size(u32)  "NOT "  count(u32)  then count x {name_off(u32), data_off(u32)}
         (offsets relative to file start, entries sorted by name for bsearch).
         ver >= 3: each name is NUL-terminated and followed by the entry size (u32 LE).
         ver <  3: size = next entry's data_off - data_off (last: total_size - data_off).

usage: ehp.py list FILE.ehp
       ehp.py extract FILE.ehp OUT_DIR
"""
import os
import struct
import sys


def read(path):
    d = open(path, "rb").read()
    if d[:3] != b"EHP":
        raise SystemExit(f"{path}: not an EHP")
    ver, total = d[3], struct.unpack_from("<I", d, 4)[0]
    count = struct.unpack_from("<I", d, 12)[0]
    ents = [struct.unpack_from("<II", d, 16 + 8 * i) for i in range(count)]
    out = []
    for i, (noff, doff) in enumerate(ents):
        name = d[noff:d.index(b"\0", noff)].decode("ascii", "replace")
        if ver >= 3:
            size = struct.unpack_from("<I", d, noff + len(name) + 1)[0]
        else:
            size = (ents[i + 1][1] if i + 1 < count else total) - doff
        out.append((name, doff, size))
    return d, out


def main():
    cmd, path = sys.argv[1], sys.argv[2]
    d, ents = read(path)
    if cmd == "list":
        for name, off, size in ents:
            print(f"{off:#010x} {size:8d} {name}")
        print(f"{len(ents)} entries")
    elif cmd == "extract":
        os.makedirs(sys.argv[3], exist_ok=True)
        for name, off, size in ents:
            open(os.path.join(sys.argv[3], name), "wb").write(d[off:off + size])
        print(f"{len(ents)} files -> {sys.argv[3]}")
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
