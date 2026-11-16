#!/usr/bin/env python3
"""Confere o status das funções em symbols/<m>.txt contra os `// @asm A-B` de src/<m>/*.c.

Inconsistências:
  matched-in-asm  função marcada `matched` cuja faixa está num `// @asm` (fica fora do asm_queue)
  asm-not-in-asm  função marcada `asm` que nenhum `// @asm` cobre (já virou C)

Uso: tools/asm_status.py [módulo...]   (sem argumentos: todos; sai com 1 se achar algo)
     tools/asm_status.py --test        (sanity check)
"""
import glob, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASM = re.compile(r'^// @asm (0x[0-9A-Fa-f]+)-(0x[0-9A-Fa-f]+)', re.M)
SYM = re.compile(r'^(\w+) = (0x[0-9A-Fa-f]+); // type:func (\w+)')


def check(sym_text, srcs):
    """srcs: {path: text}. Devolve [(tipo, nome, endereço, arquivo)]."""
    ranges = [(int(a, 16), int(b, 16), p) for p, t in srcs.items() for a, b in ASM.findall(t)]
    out = []
    for line in sym_text.splitlines():
        m = SYM.match(line)
        if not m:
            continue
        name, addr, status = m.group(1), int(m.group(2), 16), m.group(3)
        hit = next((p for a, b, p in ranges if a <= addr < b), None)
        if status == 'matched' and hit:
            out.append(('matched-in-asm', name, addr, hit))
        elif status == 'asm' and not hit:
            out.append(('asm-not-in-asm', name, addr, ''))
    return out


def test():
    sym = ('f = 0x100; // type:func matched src/m/a.c\n'
           'g = 0x200; // type:func asm (@asm in src/m/a.c)\n'
           'h = 0x300; // type:func asm (@asm in src/m/a.c)\n'
           'i = 0x400; // type:func matched src/m/a.c\n')
    src = {'src/m/a.c': '// @range 0x100-0x500\n// @asm 0x100-0x180\n// @asm 0x300-0x380\n'}
    assert check(sym, src) == [('matched-in-asm', 'f', 0x100, 'src/m/a.c'),
                               ('asm-not-in-asm', 'g', 0x200, '')], check(sym, src)
    print('ok')


def main(mods):
    if not mods:
        mods = sorted(os.path.basename(p)[:-4] for p in glob.glob(f'{ROOT}/symbols/*.txt'))
    bad = 0
    for m in mods:
        srcs = {os.path.relpath(p, ROOT): open(p, errors='replace').read()
                for p in glob.glob(f'{ROOT}/src/{m}/*.c')}
        for kind, name, addr, path in check(open(f'{ROOT}/symbols/{m}.txt').read(), srcs):
            print(f'{m}\t{kind}\t{name}\t0x{addr:08X}\t{path}')
            bad += 1
    return 1 if bad else 0


if __name__ == '__main__':
    if sys.argv[1:] == ['--test']:
        test()
    else:
        sys.exit(main(sys.argv[1:]))
