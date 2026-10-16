#!/usr/bin/env python3
"""Convert a PSP PRX (relocatable ELF, e_type 0xffa0) into a static ET_EXEC ELF.

Does what the PSP kernel does at load time: applies the PRX relocations at a
fixed base address, then rewrites headers so Ghidra/REA/splat see a plain
static MIPS ELF. Also appends the .MIPS.abiflags record REA's PSP profile
requires (PSPSDK declaration: MIPS II, 32-bit GPR, single-float).

usage: prx2elf.py IN.prx OUT.elf [--base 0x08804000]
"""
import struct
import sys

SHT_PRXRELOC = 0x700000A0
SHT_MIPS_ABIFLAGS = 0x7000002A
ET_EXEC = 2
R_MIPS_32, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16 = 2, 4, 5, 6
NID_MODULE_START = 0xD632ACDB

ABIFLAGS = struct.pack("<HBBBBBBIIII", 0, 2, 0, 1, 1, 0, 2, 0, 0, 1, 0)


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def u32(buf, off):
    return struct.unpack_from("<I", buf, off)[0]


def w32(buf, off, v):
    struct.pack_into("<I", buf, off, v & 0xFFFFFFFF)


def relocate(img, rels, base):
    """rels: list of (r_offset, type). HI16 pairs with the next LO16 in list order."""
    next_lo = [None] * len(rels)
    lo = None
    for i in range(len(rels) - 1, -1, -1):
        if rels[i][1] == R_MIPS_LO16:
            lo = rels[i][0]
        next_lo[i] = lo
    for i, (off, typ) in enumerate(rels):
        word = u32(img, off)
        if typ == R_MIPS_32:
            w32(img, off, word + base)
        elif typ == R_MIPS_26:
            target = ((word & 0x03FFFFFF) << 2) + base
            w32(img, off, (word & 0xFC000000) | ((target >> 2) & 0x03FFFFFF))
        elif typ == R_MIPS_HI16:
            if next_lo[i] is None:
                raise SystemExit(f"HI16 at {off:#x} without a following LO16")
            lo_word = u32(img, next_lo[i])
            full = ((word & 0xFFFF) << 16) + s16(lo_word & 0xFFFF) + base
            w32(img, off, (word & 0xFFFF0000) | (((full + 0x8000) >> 16) & 0xFFFF))
        elif typ == R_MIPS_LO16:
            w32(img, off, (word & 0xFFFF0000) | ((s16(word & 0xFFFF) + base) & 0xFFFF))
        else:
            raise SystemExit(f"unsupported relocation type {typ} at {off:#x}")


def find_module_start(img, secs, names, base, seg_off):
    """Walk .lib.ent -> syslib entry table -> NID module_start -> function address."""
    ent = next((s for s in secs if names[s[0]] == ".lib.ent"), None)
    if ent is None:
        return 0
    for e in range(ent[4], ent[4] + ent[5], 16):
        name_ptr, _, _, length, vcount, fcount, table = struct.unpack_from("<IHHBBHI", img, e)
        if name_ptr != 0:
            continue  # only the nameless syslib exports module_start
        table = table - base + seg_off  # vaddr -> file offset
        for k in range(fcount):
            if u32(img, table + 4 * k) == NID_MODULE_START:
                return u32(img, table + 4 * (vcount + fcount) + 4 * k)
    return 0


def convert(src, dst, base):
    img = bytearray(open(src, "rb").read())
    if img[:4] != b"\x7fELF" or struct.unpack_from("<H", img, 0x10)[0] != 0xFFA0:
        raise SystemExit(f"{src}: not a PSP PRX")
    phoff, shoff = u32(img, 0x1C), u32(img, 0x20)
    phnum, shentsize, shnum, shstrndx = (struct.unpack_from("<H", img, o)[0] for o in (0x2C, 0x2E, 0x30, 0x32))
    if phnum != 1:
        raise SystemExit(f"{src}: expected 1 program header, got {phnum}")

    secs = [list(struct.unpack_from("<IIIIIIIIII", img, shoff + i * shentsize)) for i in range(shnum)]
    strtab = secs[shstrndx]
    names = {}
    for s in secs:
        end = img.index(b"\0", strtab[4] + s[0])
        names[s[0]] = img[strtab[4] + s[0]:end].decode()

    rels = []
    for s in secs:
        if s[1] == SHT_PRXRELOC:
            for j in range(s[4], s[4] + s[5], 8):
                r_off, r_info = struct.unpack_from("<II", img, j)
                if r_info >> 8:
                    raise SystemExit(f"{src}: multi-segment relocation {r_info:#x} unsupported")
                # section data starts at file offset 0x60 == vaddr 0, so file off = vaddr + ph_offset
                rels.append((r_off + u32(img, phoff + 4), r_info & 0xFF))
            s[1] = 0  # SHT_NULL: relocations are now baked in
    relocate(img, rels, base)

    # program header: vaddr += base, keep paddr (module info file offset, PSP convention)
    w32(img, phoff + 8, u32(img, phoff + 8) + base)
    for s in secs:
        if s[2] & 2:  # SHF_ALLOC
            s[3] += base

    # append: abiflags data, new shstrtab, new section table
    abi_off = len(img)
    img += ABIFLAGS
    new_strtab = bytes(img[strtab[4]:strtab[4] + strtab[5]])
    abi_name = len(new_strtab)
    new_strtab += b".MIPS.abiflags\0"
    strtab_off = len(img)
    img += new_strtab
    strtab[4], strtab[5] = strtab_off, len(new_strtab)
    secs.append([abi_name, SHT_MIPS_ABIFLAGS, 0, 0, abi_off, 24, 0, 0, 8, 24])
    while len(img) % 4:
        img += b"\0"
    new_shoff = len(img)
    for s in secs:
        img += struct.pack("<IIIIIIIIII", *s)

    struct.pack_into("<H", img, 0x10, ET_EXEC)
    w32(img, 0x18, find_module_start(img, secs, names, base, u32(img, phoff + 4)))
    w32(img, 0x20, new_shoff)
    struct.pack_into("<H", img, 0x30, len(secs))
    open(dst, "wb").write(img)
    print(f"{dst}: {len(rels)} relocs applied at base {base:#x}, entry {u32(img, 0x18):#x}")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    base = int(sys.argv[4], 0) if len(sys.argv) > 4 and sys.argv[3] == "--base" else 0x08804000
    convert(sys.argv[1], sys.argv[2], base)
