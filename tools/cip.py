#!/usr/bin/env python3
"""Card image packs: card/cardh_<l>.cip (CIP, 4 KB slots, GIM DXT1 128x64 = card as two 64x64 halves side by side)
and card/cardm_<l>.cip (CPM, 2 KB slots, raw PSP DXT1 64x64). Slot N = card index N (CARD_IntID.bin[id-4000]).
Header 0x30: magic, u32 size, 0, 0, u32 table_size, u32 first_id, u32 last_id, u32 0x30, u32 chunks/slot; table of
{u32 chunk, u32 0} until data start = (0x30 + table_size + 0x7ff) & ~0x7ff (observed 0x6000).

usage: cip.py FILE.cip INDEX OUT.png
"""
import struct
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
import gim  # noqa: E402

d = open(sys.argv[1], "rb").read()
idx = int(sys.argv[2])
table_size, chunks = struct.unpack_from("<I", d, 0x10)[0], struct.unpack_from("<I", d, 0x20)[0]
base = (0x30 + table_size + 0x7FF) & ~0x7FF
slot = chunks * 0x800
s = d[base + idx * slot:base + (idx + 1) * slot]
if d[:3] == b"CIP":
    w, h, rgba = gim.decode(s)  # 128x64, left half = top of card, right half = bottom
    half = w // 2
    rows = [rgba[(y * w + x0) * 4:(y * w + x0 + half) * 4] for x0 in (0, half) for y in range(h)]
    open(sys.argv[3], "wb").write(gim.png(half, h * 2, b"".join(rows)))
    print(f"{sys.argv[3]}: {half}x{h * 2} (from GIM {w}x{h})")
else:
    open(sys.argv[3], "wb").write(gim.png(64, 64, gim.dxt1_psp(s, 64, 64)))
    print(f"{sys.argv[3]}: 64x64 raw DXT1")
