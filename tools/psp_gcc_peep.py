#!/usr/bin/env python3
"""psp_gcc_peep.py in.s out.s [unused-name ...] : peephole between cc1 and as of tools/psp_gcc.sh (docs/build.md, "Trilha GCC").

The SDK libraries of the game were built by a later revision of Sony's psp-gcc than the public 1.7.1. Its mips.md
clears a bit field with a two-address `ins rd,$0,pos,n` where 1.7.1 emits an AND. This pass rewrites the assembly
of cc1 1.7.1 (run with -dP, so every instruction carries its RTL, REG_DEAD notes included) to that form:

  A. `ext rd,rs,0,n` (n > 16):
     - rs still live  ->  `move rd,rs` + `ins rd,$0,n,32-n`; when rs is the word of a float taken by
       `mfc1 rs,$fN` in the same block, the copy is `mfc1 rd,$fN` again (the later revision copies first and
       masks after, and its CSE forwards the source of the copy);
     - rs dead, source and destination two different pseudo registers (the RTL names both)  ->  the copy is tied:
       `ins rs,$0,n,32-n`, and the uses of rd up to its death read rs (not done when the value outlives the
       straight-line code that follows);
     - anything else (`x &= mask` on one pseudo, a float word taken through a reload register) stays `ext`,
       as in the original (__kernel_cosf, sinf, cosf, __d2b, _dtoa_r).
  B. `andi rd,rs,M` where M has bits 15 and 0 and one run of zeros between them (an HImode `x & ~K` whose high
     bits cc1 knew to be zero; `& 0xf000` or `& 0xfffe` are left alone, they are ordinary masks more often than not)
     ->  [`move rd,rs` +] `ins rd,$0,pos,n`
  C. `li rt,M` (+ `ori rt,rt,lo`) ... `and rd,rs,rt` where ~M is one run, M is neither a low mask nor 16-bit, rt is
     used by nothing else and dies in the `and`  ->  the `li` goes away, [`move rd,rs` +] `ins rd,$0,pos,n`
  D. the objects named on the command line are dropped when they are local and nothing in the assembly refers to
     them: the `static const` that the source never uses (cc1 1.7.1 emits them, the later revision does not).
     tools/psp_gcc.sh gets the names from a second cc1 run with -Dconst= -Wunused-variable (GCC 3.3 does not warn
     about an unused `static const`, and the assembly cannot tell it from a constant that was propagated away,
     which the original does keep: the C1..C6 of kf_cos.c).

A rewrite that would change the number of instructions inside a `.set noreorder` pair (branch + delay slot) is not
done. What the pass cannot do is in docs/build.md: the later revision allocates registers and schedules around the
two-instruction form, and that is beyond a peephole.

`// @cc gcc -nopeep` on the marker line of a file turns the pass off for it (tools/psp_gcc.sh).
--check runs the sanity check."""
import re
import sys

INSN = re.compile(r'^\t([a-z][\w.]*)\t([^\t#]*)(.*)$')
REG = r'\$(\d+|sp|fp)'


def run_of(mask):
    """(pos, n) if the zero bits of the 32-bit mask are one contiguous run, else None."""
    x = ~mask & 0xFFFFFFFF
    if not x:
        return None
    pos = (x & -x).bit_length() - 1
    y = (x >> pos) + 1
    return (pos, y.bit_length() - 1) if y & (y - 1) == 0 else None


def dead(rtl, reg):
    return re.search(r'REG_DEAD \(reg[/\w]*:\w+ %s ' % reg, rtl) is not None


def peep(text, unused=()):
    L = text.split('\n')
    out = []          # output lines
    rtl = ''          # RTL comment of the instruction being read
    consts = {}       # reg -> (value, [indices in out of the lines that build it], used elsewhere)
    noreorder = False
    tie = None        # rule A, tied copy being renamed: [rd, rs, [(index in out, original line)]]
    fword = {}        # reg -> $fN while reg holds the word that `mfc1 reg,$fN` took

    def untie():
        for i, o in tie[2]:
            out[i] = o

    for l in L:
        if l.startswith(' #'):
            rtl += l + '\n'
            out.append(l)
            continue
        m = INSN.match(l)
        if not m:
            s = l.strip()
            if s == '.set\tnoreorder':
                noreorder = True
            elif s == '.set\treorder':
                noreorder = False
            if s.endswith(':') or s.startswith('.end'):
                consts = {}
                fword = {}
                if tie:
                    # the notes of an instruction moved into a delay slot are gone: at the end of the function a
                    # value that had a use is taken as dead (a wrong guess cannot go unnoticed, the function
                    # would not match)
                    if not (s.startswith('.end') and len(tie[2]) > 1):
                        untie()
                    tie = None
            if s and not s.startswith('#'):
                rtl = ''
            out.append(l)
            continue
        op, args, tail = m.group(1), [a.strip() for a in m.group(2).split(',')], m.group(3)
        new = None
        if tie:
            rd, rs = tie[0], tie[1]
            has = lambda r: re.search(re.escape(r) + r'(?!\d)', m.group(2)) is not None
            sets = re.match(r' #\((?:insn|call_insn|jump_insn)\S* \d+ \d+ \d+ \S+ \(set \(reg[/\w]*:\w+ %s ' % rd[1:], rtl) is not None
            if has(rs) or op in ('jal', 'jalr'):
                untie()
                tie = None
            elif has(rd):
                if sets and args[0] == rd:
                    l = '\t%s\t%s%s' % (op, ','.join([rd] + [re.sub(re.escape(rd) + r'(?!\d)', rs, a) for a in args[1:]]), tail)
                    tie = None
                else:
                    tie[2].append((len(out), l))
                    l = '\t%s\t%s%s' % (op, re.sub(re.escape(rd) + r'(?!\d)', rs, m.group(2)), tail)
                    if dead(rtl, rd[1:]):
                        tie = None
                m = INSN.match(l)
                op, args, tail = m.group(1), [a.strip() for a in m.group(2).split(',')], m.group(3)
        if op == 'ext' and len(args) == 4 and args[2] == '0' and int(args[3]) > 16:
            rd, rs, n = args[0], args[1], int(args[3])
            ps = re.search(r'\(set \(reg[/\w]*:\w+ \d+ \w+ \[(\d+)\]\)[\s#]+\(zero_extract:\w+ \(reg[/\w]*:\w+ \d+ \w+ \[(\d+)\]\)', rtl)
            if not dead(rtl, rs[1:]) and rd != rs:
                if not noreorder:
                    new = ['\tmove\t%s,%s' % (rd, rs), '\tins\t%s,$0,%d,%d' % (rd, n, 32 - n)]
                    if rs in fword:
                        f, at = fword[rs]
                        new[0] = '\tmfc1\t%s,%s' % (rd, f)
                        between = [x for x in out[at + 1:] if x is not None and INSN.match(x)]
                        if not any(re.search(r'\$%s(?!\d)' % r[1:], INSN.match(x).group(2)) for x in between for r in (rd, rs)):
                            # nothing touches either register in between: the masked copy is taken first
                            # (floorf: `mfc1 a1; mfc1 a2; ins a1`)
                            out[at] = new[0] + '\n' + out[at]
                            del new[0]
            elif ps and ps.group(1) != ps.group(2) and min(int(ps.group(1)), int(ps.group(2))) >= 176 and not tie:  # 176: first pseudo
                new = ['\tins\t%s,$0,%d,%d' % (rs, n, 32 - n)]
                if rd != rs:
                    tie = [rd, rs, [(len(out), l)]]
        elif op == 'andi' and len(args) == 3:
            rd, rs, imm = args[0], args[1], int(args[2], 0)
            r = run_of(imm | 0xFFFF0000) if imm & 0x8001 == 0x8001 else None
            if r and (rd == rs or not noreorder):
                new = ([] if rd == rs else ['\tmove\t%s,%s' % (rd, rs)]) + ['\tins\t%s,$0,%d,%d' % (rd, r[0], r[1])]
        elif op == 'and' and len(args) == 3:
            rd = args[0]
            for rs, rt in ((args[1], args[2]), (args[2], args[1])):
                c = consts.get(rt)
                if not c or c[2] or rs == rt:
                    continue
                val, idx, _ = c
                r = run_of(val)
                low = val & (val + 1) == 0
                if r and not low and val > 0xFFFF and (rt == rd or dead(rtl, rt[1:])) and (rd == rs or not noreorder):
                    for i in idx:
                        out[i] = None
                    new = ([] if rd == rs else ['\tmove\t%s,%s' % (rd, rs)]) + ['\tins\t%s,$0,%d,%d' % (rd, r[0], r[1])]
                    break
        # constants in registers (rule C)
        if op == 'li' and len(args) == 2 and re.match(r'-?\d+$|0x[0-9a-fA-F]+$', args[1].split()[0]) and not noreorder:
            consts[args[0]] = (int(args[1].split()[0], 0) & 0xFFFFFFFF, [len(out)], False)
        elif op == 'ori' and len(args) == 3 and args[0] == args[1] and args[0] in consts and not consts[args[0]][2] and not noreorder:
            v, idx, u = consts[args[0]]
            consts[args[0]] = (v | int(args[2], 0), idx + [len(out)], u)
        else:
            for a in args:
                for r in re.findall(r'\$(?:\d+|sp|fp)', a):
                    if r in consts:
                        if r == args[0] and op not in ('sw', 'sh', 'sb', 'beq', 'bne', 'mult', 'multu', 'div', 'divu', 'jal', 'j', 'mtc1') and not op.startswith('b'):
                            del consts[r]  # redefined
                        else:
                            consts[r] = (consts[r][0], consts[r][1], True)
            if op in ('jal', 'jalr', 'j') or op.startswith('b'):
                consts = {}
        # float words in registers (rule A)
        if op in ('jal', 'jalr'):
            fword = {}
        elif op == 'mfc1':
            fword[args[0]] = (args[1], len(out))
        elif op not in ('sw', 'sh', 'sb', 'mtc1', 'swc1') and not op.startswith('b') and op != 'j':
            fword.pop(args[0], None)
            fword = {r: f for r, f in fword.items() if f[0] not in args[:1]}
        if op in ('mtc1', 'lwc1') and len(args) > 1:
            fword = {r: f for r, f in fword.items() if f[0] != (args[1] if op == 'mtc1' else args[0])}
        if new is None:
            out.append(l)
        else:
            out.extend(new)
        rtl = ''
    return drop_unused_objects('\n'.join(x for x in out if x is not None), unused)


DATA = re.compile(r'^\t\.(word|half|byte|ascii|asciz|space|float|double|dword|align)\b')


def drop_unused_objects(text, names):
    """Rule D: `.type X,@object` blocks of `names` whose label is local and never referenced."""
    if not names:
        return text
    L = text.split('\n')
    glob = set(re.findall(r'^\t\.globl\t(\S+)', text, re.M))
    kill = set()
    i = 0
    while i < len(L):
        m = re.match(r'^\t\.type\t(\S+),\s*@object', L[i])
        if m and m.group(1) in names and m.group(1) not in glob and len(re.findall(r'(?<![\w$.])%s(?![\w$.])' % re.escape(m.group(1)), text)) == 3:
            # the three occurrences are .type, .size and the label itself
            j = i
            if j > 0 and re.match(r'^\t\.align\b', L[j - 1]):
                j -= 1
            k = i + 1
            while k < len(L) and (re.match(r'^\t\.size\b', L[k]) or L[k] == m.group(1) + ':' or (DATA.match(L[k]) and not L[k].startswith('\t.align'))):
                k += 1
            kill.update(range(j, k))
            i = k
            continue
        i += 1
    return '\n'.join(l for n, l in enumerate(L) if n not in kill)


def check():
    def code(s, *u):
        return [l for l in peep(s, *u).split('\n') if l.startswith('\t') and not l.startswith('\t.')]
    assert run_of(0xFFFFEFFF) == (12, 1) and run_of(0x00FFFFFF) == (24, 8) and run_of(0xFFFFFFF0) == (0, 4)
    assert run_of(0xFF00FF00) is None and run_of(0xFFFFFFFF) is None
    # A: source live -> move + ins; source dead -> unchanged
    live = ' #(insn 1 (set (reg:SI 3 v1) (zero_extract:SI (reg:SI 5 a1)))\n #    (nil))\n\text\t$3,$5,0,24\n'
    assert code(live) == ['\tmove\t$3,$5', '\tins\t$3,$0,24,8'], code(live)
    assert code('\tmfc1\t$6,$f12\n' + live.replace('$5', '$6')) == ['\tmfc1\t$3,$f12', '\tmfc1\t$6,$f12', '\tins\t$3,$0,24,8']
    assert code('\tmfc1\t$6,$f12\n\tsrl\t$2,$6,23\n' + live.replace('$5', '$6'))[2] == '\tmfc1\t$3,$f12'
    assert code('\tmfc1\t$6,$f12\n\tmov.s\t$f12,$f0\n' + live.replace('$5', '$6'))[2] == '\tmove\t$3,$6'
    gone = ' #(insn 1 (set (reg:SI 3 v1) (zero_extract:SI (reg:SI 5 a1)))\n #    (expr_list:REG_DEAD (reg:SI 5 a1)\n\text\t$3,$5,0,31\n'
    assert code(gone) == ['\text\t$3,$5,0,31']
    # A, tied copy: the destination's uses read the source register; given up at a label
    t = ' #(insn 1 2 3 (nil) (set (reg:SI 2 v0 [185])\n #        (zero_extract:SI (reg/v:SI 4 a0 [181])\n #    (expr_list:REG_DEAD (reg/v:SI 4 a0 [181])\n\text\t$2,$4,0,24\n'
    assert code(t + ' #(insn 2 (set (mem) (reg 2 v0))\n #    (expr_list:REG_DEAD (reg:SI 2 v0 [185])\n\tsw\t$2,72($5)\n\tmove\t$2,$6\n') == ['\tins\t$4,$0,24,8', '\tsw\t$4,72($5)', '\tmove\t$2,$6']
    assert code(t + '\tsw\t$2,72($5)\n$L3:\n') == ['\text\t$2,$4,0,24', '\tsw\t$2,72($5)']
    assert code(t + '\tj\t$31\n\tsw\t$2,72($5)\n\t.end\tf\n') == ['\tins\t$4,$0,24,8', '\tj\t$31', '\tsw\t$4,72($5)']
    assert code(t.replace('[181]', '[7]')) == ['\text\t$2,$4,0,24']
    assert code(' #(insn 1 2 3 (nil) (set (reg:SI 2 v0 [192])\n #        (zero_extract:SI (reg:SI 2 v0 [195])\n\text\t$2,$2,0,31\n') == ['\tins\t$2,$0,31,1']
    assert code(' #(insn 1 2 3 (nil) (set (reg:SI 17 s1)\n #        (zero_extract:SI (reg:SI 17 s1)\n\text\t$17,$17,0,31\n') == ['\text\t$17,$17,0,31']
    # B: HImode mask
    assert code('\tandi\t$2,$2,0xefff\n') == ['\tins\t$2,$0,12,1']
    assert code('\tandi\t$2,$3,0xefff\n') == ['\tmove\t$2,$3', '\tins\t$2,$0,12,1']
    assert code('\tandi\t$2,$2,0xf000\n') == ['\tandi\t$2,$2,0xf000']
    assert code('\tandi\t$2,$3,0x4\n') == ['\tandi\t$2,$3,0x4'] and code('\tandi\t$2,$3,0xffff\n') == ['\tandi\t$2,$3,0xffff']
    assert code('\t.set\tnoreorder\n\tj\t$L3\n\tandi\t$2,$3,0xefff\n\t.set\treorder\n')[1] == '\tandi\t$2,$3,0xefff'
    # C: li + and
    c = '\tli\t$2,-16\t\t\t# 0xfffffff0\n #(insn (and)\n #    (expr_list:REG_DEAD (reg:SI 2 v0)\n\tand\t$18,$18,$2\n'
    assert code(c) == ['\tins\t$18,$0,0,4'], code(c)
    c2 = '\tli\t$2,-16\n\taddu\t$4,$2,$5\n #(insn (and)\n #    (expr_list:REG_DEAD (reg:SI 2 v0)\n\tand\t$18,$18,$2\n'
    assert len(code(c2)) == 3  # the constant has another use
    # D: unused static const
    d = '\t.rdata\n\t.align\t2\n\t.type\tone,@object\n\t.size\tone,4\none:\n\t.word\t1\n\t.align\t2\n\t.type\ttwo,@object\n\t.size\ttwo,4\ntwo:\n\t.word\t2\n\t.text\n\tlui\t$2,%hi(two)\n'
    r = peep(d, ['one', 'two'])
    assert peep(d) == d
    assert 'one' not in r and 'two:' in r and '.word\t2' in r and '.word\t1' not in r, r
    print('psp_gcc_peep.py: sanity check OK')


if __name__ == '__main__':
    if sys.argv[1:] == ['--check']:
        check()
    elif len(sys.argv) >= 3:
        open(sys.argv[2], 'w').write(peep(open(sys.argv[1]).read(), sys.argv[3:]))
    else:
        sys.exit(__doc__)
