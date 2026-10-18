#!/usr/bin/env python3
"""Generate a splat config for a module from the relocated ELF in build/elf (sections -> segments).
usage: splat_config.py MODULE > config/MODULE.yaml
Target is the relocated static ELF (vram 0x08804000), so splat needs no PRX relocation handling;
rebuilt sections are compared byte-for-byte against the same ELF (tools/compare_sections.py)."""
import struct
import sys

m = sys.argv[1]
d = open(f"build/elf/{m}.elf", "rb").read()
shoff = struct.unpack_from("<I", d, 0x20)[0]
shnum, shstr = struct.unpack_from("<H", d, 0x30)[0], struct.unpack_from("<H", d, 0x32)[0]
secs = [struct.unpack_from("<IIIIIIIIII", d, shoff + i * 40) for i in range(shnum)]
st = secs[shstr][4]
name = lambda s: d[st + s[0]:d.index(b"\0", st + s[0])].decode()
alloc = [(name(s), s) for s in secs if s[2] & 2 and s[5] and name(s) not in (".MIPS.abiflags",)]
alloc.sort(key=lambda x: x[1][3])
text_off, text_vram = alloc[0][1][4], alloc[0][1][3]
kind = {".text": "asm", ".sceStub.text": "asm", ".data": "data", ".bss": "bss"}
# C files declare the vram range they cover with a header line `// @range 0xSTART-0xEND`; the .text
# segment is split around those ranges so the mwcc object lands exactly where the original code was.
import glob, os, re
ranges, dranges = [], []
for cf in sorted(glob.glob(f"src/{m}/*.c")):
    src = open(cf).read()
    mm = re.search(r"//\s*@range\s+0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)", src)
    if mm:
        a, b, cname = int(mm.group(1), 16), int(mm.group(2), 16), os.path.basename(cf)[:-2]
        # `// @asm 0xA-0xB`: functions inside the range that stay in asm (tools/fix_ld.py interleaves
        # the C object's per-function sections around them)
        asm = sorted((int(x.group(1), 16), int(x.group(2), 16)) for x in re.finditer(r"//\s*@asm\s+0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)", src))
        cur = a
        for x, y in asm:
            ranges.append((cur, x, cname)); ranges.append((x, y, f"{cname}_asm_{x:08X}")); cur = y
        if cur < b:  # an @asm range that ends the file leaves no trailing C segment
            ranges.append((cur, b, cname))
    for md in re.finditer(r"//\s*@data\s+0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)", src):
        dranges.append((int(md.group(1), 16), int(md.group(2), 16), os.path.basename(cf)[:-2]))
    # `// @rodata 0xA-0xB`: the object's .rodata (jump tables, const arrays). The original linker put all
    # .rodata inputs before all .data inputs inside .data, so it lands far from the file's literals; it is
    # emitted as a `.data` subsegment named <file>__rodata and tools/fix_ld.py rewrites the linker line.
    for md in re.finditer(r"//\s*@rodata\s+0x([0-9A-Fa-f]+)-0x([0-9A-Fa-f]+)", src):
        dranges.append((int(md.group(1), 16), int(md.group(2), 16), os.path.basename(cf)[:-2] + "__rodata"))
ranges.sort(); dranges.sort()
subs = []
for n, s in alloc:
    off, vram, size, typ = s[4], s[3], s[5], s[1]
    if typ == 8:  # NOBITS
        subs.append(f"      - {{ start: 0x{off:X}, type: bss, vram: 0x{vram:X}, name: bss }}")
        continue
    t = kind.get(n, "bin")
    if n == ".text" and ranges:
        cur, i = vram, 0
        for a, b, cname in ranges:
            if a > cur:
                subs.append(f"      - [0x{off + cur - vram:X}, asm, text_{i}]"); i += 1
            subs.append(f"      - [0x{off + a - vram:X}, {'asm' if '_asm_' in cname else 'c'}, {cname}]")
            cur = b
        if cur < vram + size:
            subs.append(f"      - [0x{off + cur - vram:X}, asm, text_{i}]")
        continue
    if n == ".data" and dranges:
        cur, i = vram, 0
        for a, b, cname in dranges:
            if a > cur:
                subs.append(f"      - [0x{off + cur - vram:X}, data, data_{i}]"); i += 1
            subs.append(f"      - [0x{off + a - vram:X}, .data, {cname}]")
            cur = b
        if cur < vram + size:
            subs.append(f"      - [0x{off + cur - vram:X}, data, data_{i}]")
        continue
    subs.append(f"      - [0x{off:X}, {t}, {n.strip('.').replace('.', '_')}]")
end = max(s[4] + (s[5] if s[1] != 8 else 0) for _, s in alloc)
bss_size = sum(s[5] for _, s in alloc if s[1] == 8)
print(f"""options:
  platform: psp
  compiler: GCC
  basename: {m}
  base_path: ..
  target_path: build/elf/{m}.elf
  asm_path: asm/{m}
  asset_path: assets/{m}
  src_path: src/{m}
  ld_script_path: build/{m}.ld
  symbol_addrs_path: build/{m}.syms.txt
  undefined_funcs_auto_path: build/{m}.undefined_funcs_auto.txt
  undefined_syms_auto_path: build/{m}.undefined_syms_auto.txt
  find_file_boundaries: False
  migrate_rodata_to_functions: False
  section_order: [".text", ".sceStub.text", ".data", ".rodata", ".bss"]
  ld_dependencies: False
  ld_align_section_vram_end: False
  ld_align_segment_start: 0
segments:
  - [0x0, bin, header]
  - name: {m}
    type: code
    start: 0x{text_off:X}
    vram: 0x{text_vram:X}
    bss_size: 0x{bss_size:X}
    subalign: 4
    subsegments:
{chr(10).join(subs)}
  - [0x{end:X}]
""")
