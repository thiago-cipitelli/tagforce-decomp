#!/usr/bin/env python3
"""Compare the loadable section bytes of a rebuilt ELF against the original relocated ELF.
usage: compare_sections.py ORIG.elf NEW.elf   (exit 0 when every PROGBITS section matches)"""
import hashlib
import struct
import sys


def sections(path):
    d = open(path, "rb").read()
    shoff = struct.unpack_from("<I", d, 0x20)[0]
    shnum, shstr = struct.unpack_from("<H", d, 0x30)[0], struct.unpack_from("<H", d, 0x32)[0]
    secs = [struct.unpack_from("<IIIIIIIIII", d, shoff + i * 40) for i in range(shnum)]
    st = secs[shstr][4]
    out = {}
    for s in secs:
        name = d[st + s[0]:d.index(b"\0", st + s[0])].decode()
        if s[2] & 2 and s[1] == 1:  # SHF_ALLOC, PROGBITS
            out[name] = (s[3], d[s[4]:s[4] + s[5]])
    return out


def main():
    orig, new = sections(sys.argv[1]), sections(sys.argv[2])
    # the rebuilt ELF has few big output sections: locate each original section by vram range
    def slice_new(vram, n):
        for nv, nd in new.values():
            if nv <= vram and vram + n <= nv + len(nd):
                return nd[vram - nv:vram - nv + n]
        return b""
    bad = 0
    for name, (vram, data) in sorted(orig.items(), key=lambda kv: kv[1][0]):
        if not data:
            continue
        got = slice_new(vram, len(data))
        ok = got == data
        bad += not ok
        first = next((i for i in range(min(len(got), len(data))) if got[i] != data[i]), None) if not ok else None
        print(f"{'OK ' if ok else 'BAD'} {name:24s} vram {vram:#010x} size {len(data):#8x}" + ("" if ok else f"  first diff at +{first:#x}" if first is not None else "  (length)"))
    print("MATCH" if not bad else f"{bad} section(s) differ")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
