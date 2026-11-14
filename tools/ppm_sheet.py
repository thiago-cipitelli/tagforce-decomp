#!/usr/bin/env python3
"""Frame dumps (TAGFORCE_DUMP, binary PPM) -> one PNG: a grid of the frames, half size, to look at many at once.

usage: ppm_sheet.py out.png [--cols N] [--full] frame.ppm...
  --full: frames at full size (default: every other pixel)
  --selftest: write a 2-frame sheet from synthetic frames and read the size back
"""
import struct
import sys
import zlib


def read_ppm(path):
    data = open(path, "rb").read()
    parts = data.split(maxsplit=4)
    assert parts[0] == b"P6" and parts[3] == b"255", path
    w, h = int(parts[1]), int(parts[2])
    return w, h, parts[4][: w * h * 3]


def write_png(path, w, h, rgb):
    raw = b"".join(b"\0" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))
    chunk = lambda t, d: struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d))
    open(path, "wb").write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                           + chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b""))


def sheet(out, frames, cols, step):
    imgs = [read_ppm(f) for f in frames]
    w, h = imgs[0][0] // step, imgs[0][1] // step
    rows = (len(imgs) + cols - 1) // cols
    W, H = w * cols + (cols - 1) * 2, h * rows + (rows - 1) * 2
    buf = bytearray(b"\x40" * (W * H * 3))
    for i, (fw, fh, px) in enumerate(imgs):
        ox, oy = (i % cols) * (w + 2), (i // cols) * (h + 2)
        for y in range(h):
            src = px[(y * step) * fw * 3:(y * step + 1) * fw * 3]
            row = src if step == 1 else b"".join(src[x * 3 * step:x * 3 * step + 3] for x in range(w))
            o = ((oy + y) * W + ox) * 3
            buf[o:o + w * 3] = row
    write_png(out, W, H, bytes(buf))
    return W, H


def main():
    a = sys.argv[1:]
    if a == ["--selftest"]:
        import os
        import tempfile
        d = tempfile.mkdtemp()
        for i in range(2):
            open(f"{d}/{i}.ppm", "wb").write(b"P6\n4 2\n255\n" + bytes([i * 100] * 24))
        assert sheet(f"{d}/s.png", [f"{d}/0.ppm", f"{d}/1.ppm"], 2, 1) == (10, 2)
        assert open(f"{d}/s.png", "rb").read(8) == b"\x89PNG\r\n\x1a\n"
        print("ppm_sheet: selftest ok")
        return
    cols, step = 3, 2
    if "--cols" in a:
        i = a.index("--cols")
        cols = int(a[i + 1])
        del a[i:i + 2]
    if "--full" in a:
        a.remove("--full")
        step = 1
    print(sheet(a[0], a[1:], cols, step))


if __name__ == "__main__":
    main()
