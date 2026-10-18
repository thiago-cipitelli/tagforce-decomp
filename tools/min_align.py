#!/usr/bin/env python3
"""Raise the alignment of every allocated section of an object to at least 4 bytes (rewrites in place).
The original link placed every input section on a 4-byte boundary, but data with a 16-byte aligned type
(Vec4/Mat4 templates) kept its 16; the linker script has no SUBALIGN, so this restores the 4-byte minimum.
usage: min_align.py OBJ...   (sanity check: min_align.py --test)"""
import struct, sys


def fix(path):
    d = bytearray(open(path, "rb").read())
    shoff = struct.unpack_from("<I", d, 0x20)[0]
    shnum = struct.unpack_from("<H", d, 0x30)[0]
    n = 0
    for i in range(shnum):
        off = shoff + i * 40
        flags, align = struct.unpack_from("<I", d, off + 8)[0], struct.unpack_from("<I", d, off + 32)[0]
        if flags & 2 and align < 4:  # SHF_ALLOC
            struct.pack_into("<I", d, off + 32, 4)
            n += 1
    open(path, "wb").write(d)
    return n


if __name__ == "__main__":
    if sys.argv[1:] == ["--test"]:
        import tempfile
        h = bytearray(52 + 40 * 2)
        struct.pack_into("<I", h, 0x20, 52)
        struct.pack_into("<H", h, 0x30, 2)
        struct.pack_into("<II", h, 52 + 40 + 8, 2, 0)  # section 1: ALLOC
        struct.pack_into("<I", h, 52 + 40 + 32, 1)
        f = tempfile.NamedTemporaryFile(delete=False)
        f.write(h); f.close()
        assert fix(f.name) == 1
        assert struct.unpack_from("<I", open(f.name, "rb").read(), 52 + 40 + 32)[0] == 4
        print("ok")
    else:
        for p in sys.argv[1:]:
            fix(p)
