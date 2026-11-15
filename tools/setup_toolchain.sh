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
rm -f build.tmp.c build.tmp.o build.tmp.s
"$PSPDEV/bin/psp-as" --version | head -1
echo "toolchain ok"
