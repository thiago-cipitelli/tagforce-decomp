#!/usr/bin/env python3
"""list_funcs.py [--gaps] [LO HI]: functions of $MODULE (default BOOT) that have no C yet.

Reads the split asm (asm/<module>/text_*.s, i.e. the ranges no src/ file covers; run `make MODULE=<m>`
first). Default: one line per function `ADDR SIZE NAME`, optionally limited to [LO, HI).
--gaps: one line per uncovered range `LO-HI functions bytes`, largest first.
Sanity check: `python3 tools/list_funcs.py --selftest` (needs the BOOT split)."""
import glob, os, re, sys

LINE = re.compile(r'^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/')


def functions(module):
    """[(addr, size, name, file)] for every glabel in the asm-only text segments."""
    out = []
    for path in sorted(glob.glob(f'asm/{module}/text_*.s')):
        name, start, last = None, None, None
        for l in open(path):
            if l.startswith('glabel '):
                if name and start is not None:
                    out.append((start, last + 4 - start, name, path))
                name, start = l.split()[1], None
            m = LINE.match(l)
            if m and name:
                a = int(m.group(1), 16)
                if start is None:
                    start = a
                last = a
        if name and start is not None:
            out.append((start, last + 4 - start, name, path))
    return sorted(out)


def gaps(funcs):
    by_file = {}
    for a, n, _, f in funcs:
        lo, hi, k = by_file.get(f, (a, a + n, 0))
        by_file[f] = (min(lo, a), max(hi, a + n), k + 1)
    return sorted(by_file.values(), key=lambda g: g[1] - g[0], reverse=True)


def main(argv):
    module = os.environ.get('MODULE', 'BOOT')
    if argv[:1] == ['--selftest']:
        f = functions('BOOT')
        assert f, 'no asm/BOOT/text_*.s: run make MODULE=BOOT first'
        assert all(n > 0 and n % 4 == 0 for _, n, _, _ in f)
        assert all(a1 + n1 <= a2 for (a1, n1, _, _), (a2, _, _, _) in zip(f, f[1:]))
        print('ok', len(f), 'BOOT functions without C')
        return
    funcs = functions(module)
    if not funcs:
        sys.exit(f'no asm/{module}/text_*.s: run `make MODULE={module}` first')
    if argv[:1] == ['--gaps']:
        for lo, hi, k in gaps(funcs):
            print(f'0x{lo:08X}-0x{hi:08X} {k:5d} functions {hi - lo:8d} bytes')
        return
    lo, hi = (int(argv[0], 16), int(argv[1], 16)) if len(argv) == 2 else (0, 1 << 32)
    for a, n, name, _ in funcs:
        if lo <= a < hi:
            print(f'{a:08X} {n:6d} {name}')


if __name__ == '__main__':
    main(sys.argv[1:])
