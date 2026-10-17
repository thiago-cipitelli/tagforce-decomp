#!/usr/bin/env python3
"""Rename each per-function `.text` section of an mwcc object to `.text.<function>` (and its `.rel.text`
to `.rel.text.<function>`) so the linker script can place functions individually; this is what lets a
C file keep some functions in asm (`// @asm` ranges, see tools/splat_config.py and tools/fix_ld.py).
usage: rename_sections.py OBJ   (rewrites in place)"""
import struct, sys

path = sys.argv[1]
d = bytearray(open(path, "rb").read())
shoff = struct.unpack_from("<I", d, 0x20)[0]
shnum, shstr = struct.unpack_from("<H", d, 0x30)[0], struct.unpack_from("<H", d, 0x32)[0]
secs = [list(struct.unpack_from("<IIIIIIIIII", d, shoff + i * 40)) for i in range(shnum)]
st = secs[shstr]
strtab = bytes(d[st[4]:st[4] + st[5]])
name = lambda s: strtab[s[0]:strtab.index(b"\0", s[0])].decode()
symtab = next(s for s in secs if s[1] == 2)
symstr = secs[symtab[6]]
func_of = {}
for i in range(symtab[5] // 16):
    n, v, sz, info, other, shndx = struct.unpack_from("<IIIBBH", d, symtab[4] + i * 16)
    if info & 0xF == 2 and shndx < shnum:  # STT_FUNC
        func_of[shndx] = d[symstr[4] + n:d.index(b"\0", symstr[4] + n)].decode()
new = bytearray(strtab)
renames = 0
for i, s in enumerate(secs):
    nm = name(s)
    target = i if nm == ".text" else (s[7] if nm == ".rel.text" and s[1] == 9 else None)
    if target in func_of:
        full = (".text." if nm == ".text" else ".rel.text.") + func_of[target]
        s[0] = len(new); new += full.encode() + b"\0"; renames += 1
# append the new string table at the end of the file and point shstrtab at it
st[4] = len(d); st[5] = len(new)
d += new
for i, s in enumerate(secs):
    struct.pack_into("<IIIIIIIIII", d, shoff + i * 40, *s)
open(path, "wb").write(d)
print(f"rename_sections: {path}: {renames} sections")
