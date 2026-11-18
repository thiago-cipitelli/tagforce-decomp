#!/bin/sh
# Compile a `// @cc gcc` source with the GCC the Sony SDK libraries were built with: cc1 of psp-gcc 1.7.1
# (GCC 3.3.3+allegrex-2.2.2) and the assembler of psp-gcc 1.3.1 (the modern psp-as encodes `move` as `or`, the
# original as `addu`). Both come from decomp.me's compiler collection (tools/setup_toolchain.sh; 32-bit Linux binaries).
# usage: tools/psp_gcc.sh [-DNAME...] [-I dir...] -c in.c -o out.o     (docs/build.md, "Trilha GCC")
# Flags found by matching newlib/libgcc functions of BOOT: -O2 -G0 -mext. One section per function, as the linker
# script expects (.text.<function>). The Sony libraries (libgu, libsas, libwave, libccc...) were built with -O3: `// @cc gcc -O3`.
# `// @cc gcc -fexceptions -fnon-call-exceptions`: flags after the marker are added (libgcc builds
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
MARK=$(sed -n 's|^// @cc gcc||p;20q' "$IN")
# newlib sources (include/newlib-*) take stddef.h, stdarg.h, limits.h and float.h from the compiler's own directory,
# ahead of the project's include/ (whose stdarg.h is the mwcc one)
SYS=; case "$MARK" in *include/newlib-*) SYS="-I$(dirname "$CC1")/include";; esac
S=${OUT%.o}.s
rm -f "$OUT"
# The game's SDK came from a later revision of this compiler; tools/psp_gcc_peep.py rewrites the assembly of cc1 to
# what that revision emits (field-clearing AND -> `ins rd,$0`, unused static const dropped). It reads the RTL that
# -dP writes before each instruction; the names of the unused statics come from a second cc1 run (-Dconst= -Wunused-variable). `-nopeep` on the marker line turns it off for the file.
# cc1 runs with tools/ext/psp_gcc_insand.so preloaded (rodada 37), which recognizes `x & M` (M = ones with one run
# of zeros) as one insn `ins x,$0,pos,n` tied to its source, before register allocation, as the later revision does;
# the peephole then only applies rules D, F and G (G, rodada 38: the nop Sony's assembler leaves after a 64-bit store). PSP_GCC_INSAND=0: plain cc1 1.7.1 and the peephole's rules A-C/E.
# The same .so sets n_non_fixed_regs = 192 (rodada 39: the later revision hoists more loop invariants; PSP_GCC_NNF=64 is
# cc1 1.7.1). Rodada 41: increments queued by the value of `*p++ = value` are emitted before the store (PSP_GCC_RHSQ=0: off).
# PSP_GCC_SO=path preloads another build of the hook (experiments).
PRE=; export PSP_GCC_INSAND=${PSP_GCC_INSAND:-1}
if [ "$PSP_GCC_INSAND" = 1 ]; then
  PRE=${PSP_GCC_SO:-$HERE/ext/psp_gcc_insand.so}
  [ -f "$PRE" ] || { echo "psp_gcc.sh: run tools/setup_toolchain.sh (psp_gcc_insand.so missing)" >&2; exit 1; }
fi
PEEP=1; case " $MARK " in *" -nopeep "*) PEEP=0; MARK=$(echo " $MARK " | sed 's/ -nopeep / /');; esac
# `-strsec` on the marker (rodada 43): string literals go to .rodata.str1.4 instead of .rdata, as GCC 3.4's
# default_elf_select_section does (cc1 3.3 of MIPS puts them in .rdata): the original dtoa.c object has its strings
# before .rodata.cst8 and its jump table (.rdata) after it. Applied to the assembly before `as`.
STRSEC=0; case " $MARK " in *" -strsec "*) STRSEC=1; MARK=$(echo " $MARK " | sed 's/ -strsec / /');; esac
if [ $PEEP = 1 ]; then
  LD_PRELOAD=$PRE "$CC1" -quiet -D__GNUC__=3 -D__GNUC_MINOR__=3 -D__GNUC_PATCHLEVEL__=3 -O2 -G0 -mext -ffunction-sections -w -dP -dS -dR -fsched-verbose=6 -dumpbase "$S" $MARK $SYS $FLAGS -Iinclude -I"$(dirname "$IN")" "$IN" -o "$S.cc1" \
    && UNUSED=$("$CC1" -quiet -D__GNUC__=3 -D__GNUC_MINOR__=3 -D__GNUC_PATCHLEVEL__=3 -O2 -G0 -mext -Dconst= -Wunused-variable $MARK $SYS $FLAGS -Iinclude -I"$(dirname "$IN")" "$IN" -o /dev/null 2>&1 | sed -n "s/.*warning: \`\(.*\)' defined but not used\$/\1/p") \
    && python3 "$HERE/psp_gcc_peep.py" --sched "$S.30.sched2" "$S.cc1" "$S" $UNUSED && rm -f "$S.cc1" "$S".*.sched* && "$AS" "$S" -o "$OUT" || exit 1
else
  LD_PRELOAD=$PRE "$CC1" -quiet -D__GNUC__=3 -D__GNUC_MINOR__=3 -D__GNUC_PATCHLEVEL__=3 -O2 -G0 -mext -ffunction-sections -w $MARK $SYS $FLAGS -Iinclude -I"$(dirname "$IN")" "$IN" -o "$S" && "$AS" "$S" -o "$OUT" || exit 1
fi
[ -f "$OUT" ] || exit 1
if [ $STRSEC = 1 ]; then
  python3 - "$S" <<'PY' && "$AS" "$S" -o "$OUT" || exit 1
import re, sys
# an .rdata block holding only labels, .align and .ascii (string literals) -> .rodata.str1.4
lines = open(sys.argv[1]).read().split("\n")
out = []; i = 0
while i < len(lines):
    if lines[i].strip() == ".rdata":
        j = i + 1
        while j < len(lines) and re.match(r"\s*(\$LC\d+:|\.align\s+\d+|\.ascii\s.*)$", lines[j]): j += 1
        if j > i + 1 and any(".ascii" in l for l in lines[i + 1:j]):
            out.append('\t.section\t.rodata.str1.4,"aMS",@progbits,1'); i += 1; continue
    out.append(lines[i]); i += 1
open(sys.argv[1], "w").write("\n".join(out))
PY
fi
# The original link kept every constant of .rodata.cst4/.cst8/.str1.4 (no merging across objects): clear SHF_MERGE and
# SHF_STRINGS so psp-ld leaves the sections as they are.
python3 - "$OUT" <<'PY'
import struct, sys
d = bytearray(open(sys.argv[1], "rb").read())
shoff, = struct.unpack_from("<I", d, 0x20); shnum, = struct.unpack_from("<H", d, 0x30)
for i in range(shnum):
    o = shoff + i * 40 + 8
    struct.pack_into("<I", d, o, struct.unpack_from("<I", d, o)[0] & ~0x30)
# Every function and global variable the object defines becomes a weak definition (variables: rodada 37, the state of
# nl_mallocr.c stays in the asm data under its symbols/BOOT.txt name): only the functions of the `@gcc` ranges are linked
# from it, the others are defined by the mwcc file (asm) and must win. `#pragma weak` in the source does the same,
# but at -O3 a weak function is not inlined, which changes the code of its callers.
for i in range(shnum):
    typ, _, _, off, size, link = struct.unpack_from("<6I", d, shoff + i * 40 + 4)
    if typ == 2:
        for j in range(off, off + size, 16):
            info = d[j + 12]; shndx, = struct.unpack_from("<H", d, j + 14)
            if info in (0x11, 0x12) and shndx: d[j + 12] = info + 0x10
open(sys.argv[1], "wb").write(d)
PY
