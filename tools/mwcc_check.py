#!/usr/bin/env python3
"""Compile one C file with mwccpsp and compare a function's code against the original module bytes.

Relocated fields are masked using the object's .rel.text: R_MIPS_26 compares the opcode only,
R_MIPS_HI16/LO16/GPREL16 compare the upper 16 bits only; everything else must match exactly.

usage: mwcc_check.py SRC.c FUNC MODULE ADDR [--flags "-O4,s -sdatathreshold 0"] [--build 219] [--size N]
  MODULE = name under build/elf (e.g. libhtml_view); ADDR = function address in that ELF (0x...).
"""
import argparse
import os
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
BASE, SEG = 0x08804000, 0x60
R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16 = 4, 5, 6, 7


def elf_sections(d):
    shoff = struct.unpack_from("<I", d, 0x20)[0]
    shnum, shstr = struct.unpack_from("<H", d, 0x30)[0], struct.unpack_from("<H", d, 0x32)[0]
    secs = [struct.unpack_from("<IIIIIIIIII", d, shoff + i * 40) for i in range(shnum)]
    st = secs[shstr][4]
    return {d[st + s[0]:d.index(b"\0", st + s[0])].decode(): s for s in secs}, secs


def object_function(obj, func):
    """mwcc emits one .text section per function: locate the function's section through its symbol
    and the REL section whose sh_info points at it."""
    d = open(obj, "rb").read()
    by, secs = elf_sections(d)
    symtab, strtab = by[".symtab"], by[".strtab"]
    for i in range(symtab[5] // 16):
        name, value, size, info, other, shndx = struct.unpack_from("<IIIBBH", d, symtab[4] + i * 16)
        nm = d[strtab[4] + name:d.index(b"\0", strtab[4] + name)].decode()
        if nm == func and info & 0xF == 2:  # STT_FUNC
            break
    else:
        raise SystemExit(f"{func}: not found in {obj}")
    text = secs[shndx]
    code = d[text[4] + value:text[4] + value + size]
    relocs = {}
    for rel in secs:
        if rel[1] == 9 and rel[7] == shndx:  # SHT_REL targeting the function's section
            for j in range(rel[4], rel[4] + rel[5], 8):
                off, info = struct.unpack_from("<II", d, j)
                if value <= off < value + size:
                    relocs[off - value] = info & 0xFF
    return code, relocs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src"); ap.add_argument("func"); ap.add_argument("module"); ap.add_argument("addr")
    ap.add_argument("--flags", default="-O4,p -sdatathreshold 0 -i include"); ap.add_argument("--build", default=os.environ.get("MWCC_BUILD", "219"))
    ap.add_argument("--size", type=int, default=0)
    a = ap.parse_args()
    obj = os.path.join("build", os.path.basename(a.src)[:-2] + f".{a.build}.o")
    env = dict(os.environ, MWCC_BUILD=a.build)
    r = subprocess.run([os.path.join(HERE, "mwcc.sh"), *a.flags.split(), "-c", a.src, "-o", obj], env=env, capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stdout, r.stderr); raise SystemExit("compile failed")
    code, relocs = object_function(obj, a.func)
    orig = open(os.path.join("build", "elf", a.module + ".elf"), "rb").read()
    addr = int(a.addr, 16); o = addr - BASE + SEG
    n = a.size or len(code)
    want = orig[o:o + n]
    diffs = 0
    for i in range(0, max(len(code), n), 4):
        w = struct.unpack_from("<I", code, i)[0] if i + 4 <= len(code) else None
        v = struct.unpack_from("<I", want, i)[0] if i + 4 <= len(want) else None
        t = relocs.get(i)
        if w is None or v is None:
            ok = False
        elif t == R_MIPS_26:
            ok = (w >> 26) == (v >> 26)
        elif t in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16):
            ok = (w >> 16) == (v >> 16)
        else:
            ok = w == v
        if not ok:
            diffs += 1
        mark = "  " if ok else "!!"
        print(f"{mark} {addr + i:08x}  orig {'%08x' % v if v is not None else '--------'}  new {'%08x' % w if w is not None else '--------'}" + (f"  (reloc {t})" if t else ""))
    print(f"\n{a.func}: {len(code)} B compiled vs {n} B original, {diffs} differing words, build {a.build}, flags '{a.flags}'")
    sys.exit(1 if diffs else 0)


if __name__ == "__main__":
    main()
