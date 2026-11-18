#!/bin/sh
# setup_toolchain.sh: download the external tools the build needs (Linux x86_64) and create .venv.
#   tools/ext/wibo, tools/ext/mwccpsp_219/, tools/ext/objdiff-cli, tools/ext/psp-gcc-1.3.1/, tools/ext/psp-gcc-1.7.1/   (gitignored)
#   $PSPDEV (default ~/opt/pspdev): psp-as, psp-ld, psp-objdump
# Safe to re-run: existing files are kept. Sanity check: the script ends by compiling an empty file.
set -e
cd "$(dirname "$0")/.."
EXT=tools/ext
PSPDEV=${PSPDEV:-$HOME/opt/pspdev}
mkdir -p "$EXT"
get() { [ -e "$2" ] || { echo "downloading $1"; curl -fL --progress-bar -o "$2" "$1"; }; }

get https://github.com/decompals/wibo/releases/download/1.1.0/wibo-x86_64 "$EXT/wibo"
chmod +x "$EXT/wibo"
get https://github.com/decompme/compilers/releases/download/compilers/mwccpsp_3.0.1_219.tar.gz "$EXT/mwccpsp_3.0.1_219.tar.gz"
[ -e "$EXT/mwccpsp_219/mwccpsp.exe" ] || { mkdir -p "$EXT/mwccpsp_219"; tar xzf "$EXT/mwccpsp_3.0.1_219.tar.gz" -C "$EXT/mwccpsp_219"; }
# Sony psp-gcc (GCC 3.3.3+allegrex) for the SDK code of BOOT (`// @cc gcc` files, tools/psp_gcc.sh): cc1 of 1.7.1,
# assembler of 1.3.1; 32-bit Linux binaries
for v in 1.3.1 1.7.1; do
  get https://github.com/decompme/compilers/releases/download/compilers/psp-gcc-$v.tar.gz "$EXT/psp-gcc-$v.tar.gz"
  [ -d "$EXT/psp-gcc-$v/lib" ] || { mkdir -p "$EXT/psp-gcc-$v"; tar xzf "$EXT/psp-gcc-$v.tar.gz" -C "$EXT/psp-gcc-$v"; }
done
# tools/psp_gcc_insand.c: preloaded into that cc1, it adds the later revision's field-clearing `ins` (32-bit .so,
# needs gcc -m32 and 32-bit glibc; the addresses come from the symbols of cc1)
CC1=$(ls "$EXT"/psp-gcc-1.7.1/lib/gcc-lib/psp/*/cc1)
if [ ! -e "$EXT/psp_gcc_insand.so" ] || [ tools/psp_gcc_insand.c -nt "$EXT/psp_gcc_insand.so" ]; then
  D=; for s in register_operand insn_data target_flags recog const_int_operand n_non_fixed_regs rest_of_compilation expand_expr emit_queue cfun store_expr bypass_block; do
    D="$D -DADDR_$s=0x$(nm "$CC1" | awk -v s=$s '$3 == s { print $1 }')"; done
  gcc -m32 -O2 -shared -fPIC $D tools/psp_gcc_insand.c -o "$EXT/psp_gcc_insand.so"
fi
get https://github.com/encounter/objdiff/releases/download/v3.8.2/objdiff-cli-linux-x86_64 "$EXT/objdiff-cli"
chmod +x "$EXT/objdiff-cli"

if [ ! -x "$PSPDEV/bin/psp-as" ]; then
  get https://github.com/pspdev/pspdev/releases/download/v20261001/pspdev-ubuntu-latest-x86_64.tar.gz "$EXT/pspdev.tar.gz"
  mkdir -p "$(dirname "$PSPDEV")"
  tar xzf "$EXT/pspdev.tar.gz" -C "$(dirname "$PSPDEV")"   # the tarball holds pspdev/
fi

[ -d .venv ] || python3 -m venv .venv
.venv/bin/pip install -q --disable-pip-version-check -r requirements.txt

printf 'int f(void) { return 0; }\n' > build.tmp.c
MWCIncludes=include tools/mwcc.sh -O4,p -sdatathreshold 0 -c build.tmp.c -o build.tmp.o >/dev/null
printf '// @cc gcc\nint f(void) { return 0; }\n' > build.tmp.c
tools/mwcc.sh -c build.tmp.c -o build.tmp.o
# psp_gcc_insand sanity check: `a & 0xffffff` must come out as `ins rd,$0,24,8` (cc1 1.7.1 alone emits `ext`)
printf '// @cc gcc\nunsigned f(unsigned a) { return a & 0xffffff; }\n' > build.tmp.c
tools/mwcc.sh -c build.tmp.c -o build.tmp.o
grep -q 'ins	$2,$0,24,8' build.tmp.s || { echo "psp_gcc_insand.so: no ins in build.tmp.s" >&2; exit 1; }
rm -f build.tmp.c build.tmp.o build.tmp.s
"$PSPDEV/bin/psp-as" --version | head -1
echo "toolchain ok"
