#!/usr/bin/env python3
"""sym_update.py SPEC: set names/comments in symbols/$MODULE.txt (default BOOT).

SPEC has one line per symbol: `0xADDR name comment...`, e.g.
    0x08887618 duel_08887618 type:func matched src/libduel_eng/effects_x.c
An existing line at ADDR is rewritten (its `export` and `nid:` tags are kept); new symbols are inserted
above the `// --- generated imports below` marker (or appended). Sanity check: `--selftest`."""
import os, re, sys

MARK = '// --- generated imports below'


def update(lines, specs):
    idx = {}
    for i, l in enumerate(lines):
        m = re.match(r'(\w+) = (0x[0-9A-Fa-f]+);', l)
        if m:
            idx[int(m.group(2), 16)] = i
    new = []
    for spec in specs:
        a, name, com = (spec.split(None, 2) + [''])[:3]
        a = int(a, 16)
        line = f'{name} = 0x{a:08X}; // {com}'
        if a in idx:
            old = lines[idx[a]]
            nid = re.search(r'nid:\w+', old)
            lines[idx[a]] = line + (' export' if ' export' in old else '') + (f' {nid.group(0)}' if nid else '')
        else:
            new.append(line)
    mk = [i for i, l in enumerate(lines) if l.startswith(MARK)]
    if mk:
        return lines[:mk[0]] + new + lines[mk[0]:], len(new)
    return lines + new, len(new)


if __name__ == '__main__':
    if sys.argv[1:] == ['--selftest']:
        L, n = update(['f = 0x08800000; // type:func export nid:ABC', MARK, 'imp = 0x1;'],
                      ['0x08800000 g type:func matched src/x.c', '0x08800010 h type:func'])
        assert L == ['g = 0x08800000; // type:func matched src/x.c export nid:ABC',
                     'h = 0x08800010; // type:func', MARK, 'imp = 0x1;'] and n == 1, L
        print('ok')
        sys.exit()
    path = f"symbols/{os.environ.get('MODULE', 'BOOT')}.txt"
    lines, n = update(open(path).read().rstrip('\n').split('\n'), open(sys.argv[1]).read().strip().split('\n'))
    open(path, 'w').write('\n'.join(lines) + '\n')
    print(n, 'new')
