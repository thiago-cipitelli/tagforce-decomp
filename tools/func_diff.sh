#!/bin/sh
# func_diff.sh SRC.c FUNC [LINES]: side-by-side disassembly, original | compiled from SRC.c, of a
# function named in symbols/$MODULE.txt (default BOOT). `|`, `<` and `>` mark the differences;
# relocated operands (lui/addiu of globals, jal targets) show as 0 on the right and are not real diffs.
M=${MODULE:-BOOT}
PSPDEV=${PSPDEV:-$HOME/opt/pspdev}
A=$(grep "^$2 = " symbols/$M.txt | sed 's/.*= \(0x[0-9A-Fa-f]*\).*/\1/')
[ -n "$A" ] || { echo "func_diff.sh: $2 not in symbols/$M.txt" >&2; exit 1; }
mkdir -p build/tmp
MODULE=$M python3 tools/shape_diff.py "$1" "$2" "$A" >/dev/null 2>&1 || { python3 tools/shape_diff.py "$1" "$2" "$A"; exit 1; }
for f in orig new; do
  "$PSPDEV/bin/psp-objdump" -D -b binary -mmips:allegrex -EL build/tmp/shape_$f.bin | grep -P '^\s+[0-9a-f]+:' | cut -f3- > build/tmp/full_$f.txt
done
diff -y -W 100 build/tmp/full_orig.txt build/tmp/full_new.txt | head -${3:-80}
