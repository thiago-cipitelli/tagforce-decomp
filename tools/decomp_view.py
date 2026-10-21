#!/usr/bin/env python3
"""decomp_view.py LO HI: Ghidra pseudocode of the $MODULE (default BOOT) functions in [LO, HI),
with FUN_/DAT_ names replaced by the names in symbols/<module>.txt.

Needs build/decomp/<module>/ from tools/ghidra/ExportDecomp.java (see docs/build.md). Pseudocode is a
starting point, not the truth: write the C from the asm (tools/func_asm.sh).
Sanity check: `python3 tools/decomp_view.py --selftest`."""
import os, re, sys


def load_names(module):
    s = open(f'symbols/{module}.txt').read()
    return {int(a, 16): n for n, a in re.findall(r'^(\w+) = (0x[0-9A-Fa-f]+);', s, re.M)}


def rename(text, names):
    pat = r'\b(?:FUN|ehsys|LAB|DAT|PTR_DAT)_([0-9a-fA-F]{8})\b'
    return re.sub(pat, lambda m: names.get(int(m.group(1), 16), m.group(0)), text)


def view(module, lo, hi):
    names = load_names(module)
    d = f'build/decomp/{module}'
    rows = [l.split('\t') for l in open(f'{d}/index.tsv').read().splitlines()[1:]]
    out = []
    for row in rows:
        if lo <= int(row[0], 16) < hi and os.path.exists(f'{d}/{row[1]}.c'):
            body = '\n'.join(l for l in open(f'{d}/{row[1]}.c').read().splitlines() if l.strip())
            out.append(rename(body, names))
    return out


if __name__ == '__main__':
    if sys.argv[1:] == ['--selftest']:
        assert rename('FUN_0884d7c4(DAT_12345678)', {0x0884D7C4: 'card_get_name'}) == 'card_get_name(DAT_12345678)'
        print('ok')
    else:
        print('\n'.join(view(os.environ.get('MODULE', 'BOOT'), int(sys.argv[1], 16), int(sys.argv[2], 16))))
