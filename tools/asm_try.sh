#!/bin/sh
# asm_try.sh FUNC: compile the file that holds FUNC with -DNON_MATCHING and compare FUNC against the original,
# printing only the differing words (relocated operands are masked) and the summary line. Cheap on output:
# the inner loop of the matching work (docs/matching.md). MODULE env as in the other tools (default BOOT).
# usage: MODULE=libfield tools/asm_try.sh fbm_icon      sanity: tools/asm_try.sh --check
if [ "$1" = "--check" ]; then  # a matched BOOT function must report 0 differing words
    out=$(MODULE=BOOT "$0" anim_eval) || exit 1
    echo "$out" | grep -q ", 0 differing words" && echo "asm_try.sh: ok" && exit 0
    echo "asm_try.sh: sanity check failed: $out" >&2; exit 1
fi
M=${MODULE:-BOOT}
L=$(grep "^$1 = " symbols/$M.txt)
[ -n "$L" ] || { echo "asm_try.sh: $1 not in symbols/$M.txt" >&2; exit 1; }
A=$(echo "$L" | sed 's/.*= \(0x[0-9A-Fa-f]*\).*/\1/')
F=$(echo "$L" | grep -o 'src/[A-Za-z0-9_/]*\.c' | head -1)
[ -n "$F" ] || F=$(grep -l "^[a-zA-Z].*[ *]$1(" src/$M/*.c | head -1)
[ -n "$F" ] || { echo "asm_try.sh: no source file for $1" >&2; exit 1; }
python3 tools/mwcc_check.py "$F" "$1" "$M" "$A" --flags "-O4,p -sdatathreshold 0 -i include -DNON_MATCHING" 2>&1 |
    awk '/^!!/ { print; next } /orig [0-9a-f]+ +new [0-9a-f]+/ { if ($3 != $5 && $0 !~ /reloc/) print; next } { print }'
