#!/bin/sh
# func_asm.sh FUNC: asm of FUNC from the split asm of $MODULE (default BOOT), without the
# addresses and the $s/$ra saves and restores. Functions not yet in src/ keep their splat name
# (func_XXXXXXXX) until `make` re-splits. Sanity check: MODULE=libhtml_view tools/func_asm.sh <any glabel>
M=${MODULE:-BOOT}
f=$(grep -l "glabel $1\$" asm/$M/*.s 2>/dev/null | head -1)
[ -n "$f" ] || { echo "func_asm.sh: no 'glabel $1' in asm/$M/ (still func_XXXXXXXX? run make MODULE=$M?)" >&2; exit 1; }
awk "/glabel $1\$/,/endlabel $1\$/" "$f" | cut -c30- | grep -v '^$' | sed 's/^[0-9A-F] \*\/  //' \
  | grep -v '[sl]w         \$\(s[0-8]\|fp\|ra\), 0x..(\$sp)'
