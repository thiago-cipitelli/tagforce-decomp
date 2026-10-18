#!/usr/bin/env python3
"""Make splat's output assemble with psp-as: some VFPU forms spimdisasm prints (vrot.* rotation lists, vpfxd masks)
use a syntax psp-as rejects. Assemble, collect the failing lines and replace each with `.word 0x<hex>` taken from
the instruction comment (`/* off vram HEX */`, bytes in file order), which is byte-identical. usage: fix_asm.py AS [asflags...] -- file.s..."""
import re, subprocess, sys
i = sys.argv.index("--")
as_cmd, files = sys.argv[1:i], sys.argv[i + 1:]
def words(line):
    # splat aligns 8-byte data to 8 (`.align 3`); the original link only had 4-byte alignment there, and
    # the linker script no longer forces SUBALIGN(4) (C objects need their own 16-byte alignment).
    if line.strip() == ".align 3":
        return ".align 2"
    # `.double`/`.dword` make psp-as insert 8-byte alignment padding the original link never had
    # (the data section only starts 4-aligned); emit the same bytes as two `.word`.
    m = re.match(r"(\s*/\* \w+ \w+ ([0-9A-Fa-f]{16}) \*/)\s+\.(double|dword)\s", line)
    if not m:
        return line
    b = bytes.fromhex(m.group(2))
    return f"{m.group(1)} .word 0x{b[:4][::-1].hex().upper()}, 0x{b[4:][::-1].hex().upper()}"


for f in files:
    src = open(f).read().split("\n")
    fixed = [words(l) for l in src]
    if fixed != src:
        open(f, "w").write("\n".join(fixed))
        print(f"fix_asm: {f}: {sum(a != b for a, b in zip(src, fixed))} 8-byte data directives rewritten as .word")
    for _ in range(3):
        r = subprocess.run(as_cmd + ["-o", "/dev/null", f], capture_output=True, text=True)
        bad = sorted({int(m) for m in re.findall(rf"{re.escape(f)}:(\d+): Error", r.stderr)})
        if not bad:
            break
        lines = open(f).read().split("\n")
        n = 0
        for ln in bad:
            m = re.match(r"(\s*/\* \w+ \w+ ([0-9A-Fa-f]{8}) \*/)\s+\S.*", lines[ln - 1])
            if m:
                le = bytes.fromhex(m.group(2))[::-1].hex().upper()  # comment shows file byte order
                lines[ln - 1] = f"{m.group(1)}  .word 0x{le}"; n += 1
        open(f, "w").write("\n".join(lines))
        print(f"fix_asm: {f}: {n}/{len(bad)} failing lines replaced by .word")
        if n == 0:
            sys.exit(r.stderr)
