#!/bin/sh
# Compile a `// @cc gcc` source with the GCC the Sony SDK libraries were built with: cc1 of psp-gcc 1.7.1
# (GCC 3.3.3+allegrex-2.2.2) and the assembler of psp-gcc 1.3.1 (the modern psp-as encodes `move` as `or`, the
# original as `addu`). Both come from decomp.me's compiler collection (tools/setup_toolchain.sh; 32-bit Linux binaries).
# usage: tools/psp_gcc.sh [-DNAME...] [-I dir...] -c in.c -o out.o     (docs/build.md, "Trilha GCC")
# Flags found by matching newlib/libgcc functions of BOOT: -O2 -G0 -mext. One section per function, as the linker
# script expects (.text.<function>). `// @cc gcc -fexceptions -fnon-call-exceptions`: flags after the marker are added (libgcc builds
# its division functions with those two).
HERE=$(cd "$(dirname "$0")" && pwd)
CC1=$(ls "$HERE"/ext/psp-gcc-1.7.1/lib/gcc-lib/psp/*/cc1 2>/dev/null)
AS=$HERE/ext/psp-gcc-1.3.1/bin/psp-as
[ -x "$CC1" ] && [ -x "$AS" ] || { echo "psp_gcc.sh: run tools/setup_toolchain.sh (psp-gcc 1.7.1 / 1.3.1 missing)" >&2; exit 1; }
FLAGS=; IN=; OUT=
while [ $# -gt 0 ]; do
  case "$1" in
    -c) IN=$2; shift;; -o) OUT=$2; shift;;
    -i|-I) FLAGS="$FLAGS -I$2"; shift;;
    -D*|-I*|-U*|-f*) FLAGS="$FLAGS $1";;
  esac; shift
done
# extra flags of this file: the rest of its `// @cc gcc` line
FLAGS="$FLAGS $(sed -n 's|^// @cc gcc||p;20q' "$IN")"
S=${OUT%.o}.s
rm -f "$OUT"
"$CC1" -quiet -D__GNUC__=3 -D__GNUC_MINOR__=3 -D__GNUC_PATCHLEVEL__=3 -O2 -G0 -mext -ffunction-sections -w $FLAGS -Iinclude -I"$(dirname "$IN")" "$IN" -o "$S" && "$AS" "$S" -o "$OUT" || exit 1
[ -f "$OUT" ] || exit 1
# The original link kept every constant of .rodata.cst4/.cst8/.str1.4 (no merging across objects): clear SHF_MERGE and
# SHF_STRINGS so psp-ld leaves the sections as they are.
python3 - "$OUT" <<'PY'
import struct, sys
d = bytearray(open(sys.argv[1], "rb").read())
shoff, = struct.unpack_from("<I", d, 0x20); shnum, = struct.unpack_from("<H", d, 0x30)
for i in range(shnum):
    o = shoff + i * 40 + 8
    struct.pack_into("<I", d, o, struct.unpack_from("<I", d, o)[0] & ~0x30)
open(sys.argv[1], "wb").write(d)
PY
