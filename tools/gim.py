#!/usr/bin/env python3
"""GIM (Sony PSP "MIG.00.1PSP") texture decoder -> PNG, stdlib only.

Block chain after the 16-byte magic: {u16 id, u16 _, u32 size, u32 next, u32 data_off(0x10)}.
ids: 2 root, 3 picture (containers), 4 image, 5 palette, 0xFF file info.
Image/palette header at block+0x10: u16 hdr_size(0x30) u16 _ u16 format u16 order u16 w u16 h u16 bpp
u16 pitch_align u16 height_align u16 _ u32 _ u32 level_table_off u32 data_off ... ; pixels at block+0x10+data_off.
formats: 0 RGBA5650 1 RGBA5551 2 RGBA4444 3 RGBA8888 4 INDEX4 5 INDEX8 6 INDEX16 7 INDEX32 8 DXT1(PSP). order 1 = PSP swizzle.

usage: gim.py FILE.gim [OUT.png]
"""
import struct
import sys
import zlib


def blocks(d):
    off, out = 0x10, []
    while off + 16 <= len(d):
        bid, _, size, nxt, dof = struct.unpack_from("<HHIII", d, off)
        if size == 0:
            break
        out.append((bid, off))
        off += 16 if bid in (2, 3) else size
    return out


def header(d, blk):
    h = blk + 0x10
    fmt, order, w, hgt, bpp, pitch_align, height_align = struct.unpack_from("<HHHHHHH", d, h + 4)
    data_off = struct.unpack_from("<I", d, h + 0x1C)[0]
    return fmt, order, w, hgt, bpp, pitch_align, height_align, h + data_off


def unswizzle(buf, pitch, height):
    out = bytearray(len(buf))
    bw, bh = pitch // 16, height // 8
    i = 0
    for by in range(bh):
        for bx in range(bw):
            for y in range(8):
                dst = (by * 8 + y) * pitch + bx * 16
                out[dst:dst + 16] = buf[i:i + 16]
                i += 16
    return bytes(out)


def c565(v):
    return ((v >> 11 & 31) * 255 // 31, (v >> 5 & 63) * 255 // 63, (v & 31) * 255 // 31)


def dxt1_psp(buf, w, h):
    """PSP DXT1: per 4x4 block, u32 indices FIRST, then u16 color0, u16 color1 (PC order is reversed)."""
    out = bytearray(w * h * 4)
    bw = w // 4
    for bi in range(min(len(buf) // 8, bw * (h // 4))):
        idx, c0, c1 = struct.unpack_from("<IHH", buf, bi * 8)
        r0, g0, b0 = c565(c0)
        r1, g1, b1 = c565(c1)
        if c0 > c1:
            pal = [(r0, g0, b0, 255), (r1, g1, b1, 255), ((2 * r0 + r1) // 3, (2 * g0 + g1) // 3, (2 * b0 + b1) // 3, 255),
                   ((r0 + 2 * r1) // 3, (g0 + 2 * g1) // 3, (b0 + 2 * b1) // 3, 255)]
        else:
            pal = [(r0, g0, b0, 255), (r1, g1, b1, 255), ((r0 + r1) // 2, (g0 + g1) // 2, (b0 + b1) // 2, 255), (0, 0, 0, 0)]
        bx, by = (bi % bw) * 4, (bi // bw) * 4
        for py in range(4):
            for px in range(4):
                o = ((by + py) * w + bx + px) * 4
                out[o:o + 4] = bytes(pal[(idx >> (2 * (py * 4 + px))) & 3])
    return bytes(out)


def px_rgba(fmt, v):
    if fmt == 0:
        return ((v & 31) * 255 // 31, ((v >> 5) & 63) * 255 // 63, ((v >> 11) & 31) * 255 // 31, 255)
    if fmt == 1:
        return ((v & 31) * 255 // 31, ((v >> 5) & 31) * 255 // 31, ((v >> 10) & 31) * 255 // 31, 255 if v >> 15 else 0)
    if fmt == 2:
        return ((v & 15) * 17, ((v >> 4) & 15) * 17, ((v >> 8) & 15) * 17, (v >> 12) * 17)
    return (v & 255, (v >> 8) & 255, (v >> 16) & 255, (v >> 24) & 255)


def decode(d):
    img = pal = None
    for bid, blk in blocks(d):
        if bid == 4:
            img = header(d, blk)
        elif bid == 5:
            pal = header(d, blk)
    if img is None:
        raise SystemExit("no image block")
    fmt, order, w, h, bpp, pa, ha, data = img
    if fmt == 8:
        return w, h, dxt1_psp(d[data:], w, h)
    pitch = (w * bpp // 8 + pa - 1) // pa * pa
    rows = (h + ha - 1) // ha * ha
    buf = d[data:data + pitch * rows]
    if order == 1:
        buf = unswizzle(buf, pitch, rows)
    palette = None
    if pal is not None:
        pf, _, pw, ph, pbpp, _, _, pdata = pal
        step = pbpp // 8
        palette = [px_rgba(pf, int.from_bytes(d[pdata + i * step:pdata + (i + 1) * step], "little")) for i in range(pw * ph)]
    out = bytearray()
    for y in range(h):
        row = buf[y * pitch:(y + 1) * pitch]
        for x in range(w):
            if fmt == 4:
                v = (row[x // 2] >> (4 * (x & 1))) & 15
            elif fmt == 5:
                v = row[x]
            elif fmt in (6, 7):
                s = 2 if fmt == 6 else 4
                v = int.from_bytes(row[x * s:(x + 1) * s], "little")
            else:
                s = bpp // 8
                v = int.from_bytes(row[x * s:(x + 1) * s], "little")
            out += bytes(palette[v] if fmt >= 4 else px_rgba(fmt, v))
    return w, h, bytes(out)


def png(w, h, rgba):
    raw = b"".join(b"\0" + rgba[y * w * 4:(y + 1) * w * 4] for y in range(h))
    def chunk(t, b):
        return struct.pack(">I", len(b)) + t + b + struct.pack(">I", zlib.crc32(t + b) & 0xFFFFFFFF)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)) + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b"")


if __name__ == "__main__":
    d = open(sys.argv[1], "rb").read()
    w, h, rgba = decode(d)
    out = sys.argv[2] if len(sys.argv) > 2 else sys.argv[1].rsplit(".", 1)[0] + ".png"
    open(out, "wb").write(png(w, h, rgba))
    print(f"{out}: {w}x{h}")
