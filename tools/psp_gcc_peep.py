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

  E. (--sched DUMP) the blocks where A-C changed the number of instructions are scheduled again: the later
     revision's AND is one insn up to reload, which adds the copy, and sched2 places the copy and the `ins` apart.
     resched() replays GCC 3.3's list scheduler with the dependences, priorities, costs and units of cc1's own
     sched2 dump (-dR -fsched-verbose=6), and the dbr pass as "the insn just before a branch fills its slot". A
     block is only reordered when the replay of the unchanged block gives cc1's order. The tied copy of rule A
     keeps the count and is not rescheduled (its effect on -frename-registers is not modelled).

  F. `ulw`/`usw` become the `.word` of Sony's lwl/lwr (swl/swr) pair (psp-as 1.3.1 rejects lwl/lwr on allegrex).

A rewrite that would change the number of instructions inside a `.set noreorder` pair (branch + delay slot) is not
done. What the pass cannot do is in docs/build.md: the later revision allocates registers and schedules around the
two-instruction form, and that is beyond a peephole.

Since round 37 tools/psp_gcc.sh preloads tools/psp_gcc_insand.so, which gives cc1 the later revision's `ins` itself;
in that mode (PSP_GCC_INSAND=1, the default) only rules D and F run. Rules A-C/E are the fallback for PSP_GCC_INSAND=0.

`// @cc gcc -nopeep` on the marker line of a file turns the pass off for it (tools/psp_gcc.sh).
--check runs the sanity check."""
import collections
import os
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


def uid_of(line):
    """insn UID that cc1 -dP writes after an instruction (`\t # 17\tpattern\t[length = 4]`), '' if none"""
    m = re.search(r'\s#\s+(\d+[abm]?)(?=\s|$)(?!.*\s#\s+\d)', line)
    return m.group(1) if m else ''


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
        mtag = False
        if op == 'ext' and len(args) == 4 and args[2] == '0' and int(args[3]) > 16:
            rd, rs, n = args[0], args[1], int(args[3])
            ps = re.search(r'\(set \(reg[/\w]*:\w+ \d+ \w+ \[(\d+)\]\)[\s#]+\(zero_extract:\w+ \(reg[/\w]*:\w+ \d+ \w+ \[(\d+)\]\)', rtl)
            if not dead(rtl, rs[1:]) and rd != rs:
                if not noreorder:
                    new = ['\tmove\t%s,%s' % (rd, rs), '\tins\t%s,$0,%d,%d' % (rd, n, 32 - n)]
                    if rs in fword:
                        mtag = True
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
                        out[i] = '\t#gone ' + uid_of(out[i])
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
            # tag the lines for resched(): `# U` same count, `# Ua`/`# Ub` the copy and the ins of insn U
            u = uid_of(l)
            if not u:
                pass
            elif mtag:
                new = [x + '\t # %sm' % u for x in new]
            elif len(new) == 2:
                new = [new[0] + '\t # %sa' % u, new[1] + '\t # %sb' % u]
            else:
                new = [new[0] + '\t # %s' % u]
            out.extend(new)
        rtl = ''
    return unaligned(drop_unused_objects('\n'.join(x for x in out if x is not None), unused))


ULW = re.compile(r'^\s*(ulw|usw)\s+\$(\d+),(-?\d+)\(\$(\d+)\)(.*)$')
LWL = re.compile(r'^\s*(lwl|lwr|swl|swr)\s+\$(\w+),(-?\d+)\(\$(\w+)\)(.*)$')
REGNO = {'zero': 0, 'at': 1, 'gp': 28, 'sp': 29, 'fp': 30, 'ra': 31}
regno = lambda r: int(r) if r.isdigit() else REGNO[r]
LWL_OP = {'lwl': 0x22, 'lwr': 0x26, 'swl': 0x2A, 'swr': 0x2E}


def unaligned(text):
    """Rule F. `ulw`/`usw` (newlib's machine/mips memcpy): Sony's assembler expands them to `lwl rt,o+3(b)` +
    `lwr rt,o(b)` (`swl`/`swr`); psp-as 1.3.1 refuses lwl/lwr on the allegrex and expands to byte accesses, so
    the pair is written as .word. Plain lwl/lwr/swl/swr that cc1 emits (unaligned struct copies) likewise."""
    out = []
    for l in text.split('\n'):
        m = LWL.match(l)   # unaligned struct copies cc1 emits itself (gzip header/trailer)
        if m:
            w = LWL_OP[m.group(1)] << 26 | regno(m.group(4)) << 21 | regno(m.group(2)) << 16 | (int(m.group(3)) & 0xFFFF)
            out.append('\t.word\t0x%08x\t# %s $%s,%s($%s)%s' % (w, m.group(1), m.group(2), m.group(3), m.group(4), m.group(5)))
            continue
        m = ULW.match(l)
        if m:
            hi, lo = (0x22, 0x26) if m.group(1) == 'ulw' else (0x2A, 0x2E)
            rt, o, b = int(m.group(2)), int(m.group(3)), int(m.group(4))
            enc = lambda op, off: (op << 26) | (b << 21) | (rt << 16) | (off & 0xFFFF)
            out += ['\t.word\t0x%08x\t# %sl $%d,%d($%d)%s' % (enc(hi, o + 3), m.group(1)[1], rt, o + 3, b, m.group(5)),
                    '\t.word\t0x%08x\t# %sr $%d,%d($%d)' % (enc(lo, o), m.group(1)[1], rt, o, b)]
        else:
            out.append(l)
    return '\n'.join(out)


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


# ---------------------------------------------------------------------------------------------------------------
# E. Rescheduling. The later revision makes the field-clearing AND a two-address insn: before reload it is one insn
# (register allocation is the one of cc1 1.7.1's `ext`), reload adds the copy (`move rd,rs`) when rs stays live,
# and sched2 then schedules the copy and the `ins` as two insns. cc1 1.7.1 scheduled one. resched() replays sched2
# (GCC 3.3 haifa list scheduler) on every block where rules A-C changed the instruction count, with the dependence
# graph, priorities, costs and units cc1 wrote to its sched2 dump (-dR -fsched-verbose), and reorders the block.
# A block is only touched when replaying the unchanged graph gives exactly the order cc1 chose, and the delay slots
# of the block were filled by the insn just before each branch (the dbr pass is replayed that way).

def parse_sched2(text):
    """[(function, rows, order, luids)] per block scheduled after reload; rows[uid] = prio, cost, unit, code, succs"""
    B = []
    fn = cur = None
    for l in text.split('\n'):
        m = re.match(r';; Function (\S+)', l)
        if m:
            fn = m.group(1)
            continue
        if '-- basic block' in l:
            cur = (fn, {}, [], []) if 'after reload' in l else None
            if cur:
                B.append(cur)
            continue
        if cur is None:
            continue
        m = re.match(r';;\s+(\d+)\s+(-?\d+)\s+\d+\s+\d+\s+(\d+)\s+(\d+)\s+\d+ -\s*(\d+)\s+(\w+)\s*:(.*)', l)
        if m:
            u = m.group(1)
            cur[1][u] = dict(code=int(m.group(2)), prio=int(m.group(3)), cost=int(m.group(4)), bmax=int(m.group(5)),
                             unit=m.group(6), succs=m.group(7).split())
            cur[3].append(u)
            continue
        m = re.search(r'--> scheduling insn <<<(\d+)>>>', l)
        if m:
            cur[2].append(m.group(1))
    return B


STORE = re.compile(r's(b|h|w|wc1|dc1|wl|wr)$')


def list_schedule(rows, luids, kinds, prio, maxblk, op):
    """haifa-sched of GCC 3.3, one issue per cycle, function units with the blockage of their last insn"""
    luid = {u: i for i, u in enumerate(luids)}
    succ = {u: [s for s in rows[u]['succs'] if s in luid] for u in luids}

    def cost(a, b):
        if rows[a]['code'] < 0 or rows[b]['code'] < 0 or kinds.get((a, b), 'TRUE') != 'TRUE':
            return 0  # a USE costs nothing; anti and output dependences cost nothing on MIPS
        return rows[a]['cost']
    depc = collections.Counter(s for u in luids for s in succ[u])
    tick = collections.Counter()
    ready = [u for u in luids if not depc[u]]
    queue = collections.defaultdict(list)
    clock, last, order, utick, ulast = -1, None, [], {}, {}

    def klass(u):
        if last is None or u not in succ[last] or cost(last, u) == 1:
            return 3
        return 1 if kinds.get((last, u), 'TRUE') == 'TRUE' else 2
    while len(order) < len(luids):
        clock += 1
        ready += queue.pop(clock, [])
        if not ready:
            if not queue:
                return None
            clock = min(queue)
            ready += queue.pop(clock)
        ready.sort(key=lambda u: (prio[u], klass(u), len(succ[u]), -luid[u]))
        issued = False
        while ready and not issued:
            u = ready.pop()
            un = rows[u]['unit']
            if un != 'none' and un in utick:
                t = utick[un]
                if t > clock:
                    t += (3 if STORE.match(op.get(ulast[un], '')) else max(1, rows[ulast[un]]['cost'] - rows[u]['cost'] + 1)) - maxblk[un]
                    if t > clock:
                        queue[t].append(u)
                        continue
            if un != 'none':
                utick[un], ulast[un] = clock + maxblk[un], u
            order.append(u)
            last, issued = u, rows[u]['code'] >= 0
            for s in succ[u]:
                tick[s] = max(tick[s], clock + cost(u, s))
                depc[s] -= 1
                if not depc[s]:
                    if tick[s] - clock < 1:
                        ready.insert(0, s)
                    else:
                        queue[tick[s]].append(s)
    return order


def formula_prio(rows, luids, kinds):
    luid = set(luids)
    P = {}

    def pr(u):
        if u not in P:
            ss = [s for s in rows[u]['succs'] if s in luid]
            c = lambda s: 0 if rows[u]['code'] < 0 or rows[s]['code'] < 0 or kinds.get((u, s), 'TRUE') != 'TRUE' else rows[u]['cost']
            P[u] = max(c(s) + pr(s) for s in ss) if ss else max(rows[u]['cost'], 0) if rows[u]['code'] >= 0 else 0
        return P[u]
    for u in luids:
        pr(u)
    return P


BRANCH = re.compile(r'(b\w*|j|jr|jal|jalr)$')


def regs(args):
    return set(re.findall(r'\$(?:\d+|sp|fp|f\d+)', ','.join(args)))


def resched(text, dump):
    debug = os.environ.get('PEEP_DEBUG')
    B = parse_sched2(dump)
    L = text.split('\n')
    maxblk = collections.defaultdict(int)
    for b in B:
        for r in b[1].values():
            maxblk[r['unit']] = max(maxblk[r['unit']], r['bmax'])
    funcs = {}   # name -> (start, end) line range
    for i, l in enumerate(L):
        m = re.match(r'\t\.ent\t(\S+)', l)
        if m:
            funcs[m.group(1)] = [i, None] if m.group(1) not in funcs else None
        m = re.match(r'\t\.end\t(\S+)', l)
        if m and funcs.get(m.group(1)):
            funcs[m.group(1)][1] = i
    for fn, rows, order, luids in B:
        if not funcs.get(fn) or not order:
            continue
        a, z = funcs[fn]
        units = {}   # tag -> line index
        kinds = {}
        op = {}
        args = {}
        rtl = []
        gone = set()
        for i in range(a, z):
            l = L[i]
            if l is None:
                continue
            if l.startswith(' #'):
                rtl.append(l[2:])
                continue
            m = re.match(r'\t#gone (\d+)$', l)
            if m:
                gone.add(m.group(1))
                continue
            m = INSN.match(l)
            if m and not l.startswith('\t.'):
                u = uid_of(l)
                if u:
                    units[u] = i
                    op[u], args[u] = m.group(1), [x.strip() for x in m.group(2).split(',')]
                    r = ' '.join(x.strip() for x in rtl)
                    ml = re.search(r'\} (\(insn_list.*?|\(nil\)) *(\(expr_list|\(nil\)|$)', r)
                    if ml and re.match(r'\((?:insn|jump_insn|call_insn)\S* %s ' % u, r):
                        for k, p in re.findall(r'insn_list(?::REG_DEP_(\w+))? (\d+)', ml.group(1)):
                            kinds[(p, u)] = k or 'TRUE'
                rtl = []
            elif l.strip() and not l.startswith('\t.set') and not l.startswith('\t#'):
                rtl = []
        split = [u for u in luids if u + 'a' in units]
        dropped = [u for u in luids if u in gone]
        if not split and not dropped or any(u + 'm' in units for u in luids):
            continue
        # the block as it is in the text: every insn of the dump present, nothing else in between
        def tags(u):
            return [u + 'a', u + 'b'] if u in split else [] if u in dropped or rows[u]['code'] < 0 else [u]
        want = [t for u in luids for t in tags(u)]
        if any(t not in units for t in want):
            if debug: sys.stderr.write('resched %s %s: skip 1 %s\n' % (fn, luids[0], [t for t in want if t not in units]))
            continue
        idx = sorted(units[t] for t in want)
        lo, hi = idx[0], idx[-1]
        while L[hi + 1] in ('\t.set\tmacro', '\t.set\treorder'):
            hi += 1
        if any(L[i] is None for i in range(lo, hi + 1)) \
                or any(INSN.match(L[i]) and not L[i].startswith('\t.') and uid_of(L[i]) not in want for i in range(lo, hi + 1)) \
                or any(re.match(r'^\$?\w+:$', L[i]) for i in range(lo + 1, hi + 1)):
            if debug: sys.stderr.write('resched %s %s: skip 2\n' % (fn, luids[0]))
            continue
        # text order -> order before dbr: each filled delay slot came from just before its branch
        seq = [uid_of(L[i]) for i in range(lo, hi + 1) if INSN.match(L[i]) and not L[i].startswith('\t.')]
        noreo = set()
        nr = False
        for i in range(lo, hi + 1):
            s = L[i].strip()
            if s == '.set\tnoreorder':
                nr = True
            elif s == '.set\treorder':
                nr = False
            elif INSN.match(L[i]) and not L[i].startswith('\t.') and nr:
                noreo.add(uid_of(L[i]))
        pre = []
        slots = {}   # branch -> slot
        k = 0
        while k < len(seq):
            t = seq[k]
            if t in noreo and BRANCH.match(op[t]) and k + 1 < len(seq) and seq[k + 1] in noreo:
                pre += [seq[k + 1], t]
                slots[t] = seq[k + 1]
                k += 2
            else:
                pre.append(t)
                k += 1
        if any(t in noreo and t not in slots and t not in slots.values() for t in seq):
            if debug: sys.stderr.write('resched %s %s: skip 3\n' % (fn, luids[0]))
            continue
        if pre != [t for u in order for t in tags(u)]:
            if debug: sys.stderr.write('resched %s %s: skip 4\n' % (fn, luids[0]))
            continue
        P0 = formula_prio(rows, luids, kinds)
        if list_schedule(rows, luids, kinds, {u: rows[u]['prio'] for u in luids}, maxblk, op) != order:
            if debug: sys.stderr.write('resched %s %s: skip 5\n' % (fn, luids[0]))
            continue
        # the graph of the later revision: U -> Ua (copy) + Ub (ins), dropped constants removed
        rows2 = {}
        kinds2 = dict(kinds)
        luids2 = []
        for u in luids:
            if u in dropped:
                if debug: sys.stderr.write('resched %s %s: skip 6\n' % (fn, luids[0]))
                continue
            r = dict(rows[u], succs=[t for s in rows[u]['succs'] if s not in dropped for t in ([s + 'a'] if s in split else [s])])
            if u in split:
                last = luids[-1]
                rows2[u + 'a'] = dict(r, succs=[u + 'b'] + [s for s in r['succs'] if kinds.get((u, s.rstrip('a')), 'TRUE') == 'ANTI' or s == last])
                rows2[u + 'b'] = dict(r, succs=[s for s in r['succs'] if kinds.get((u, s.rstrip('a')), 'TRUE') != 'ANTI' or s == last])
                for s in r['succs']:
                    kinds2[(u + 'a', s)] = kinds2[(u + 'b', s)] = kinds.get((u, s.rstrip('a')), 'TRUE')
                kinds2[(u + 'a', u + 'b')] = 'TRUE'
                luids2 += [u + 'a', u + 'b']
            else:
                rows2[u] = r
                luids2.append(u)
            for s in rows[u]['succs']:
                if s in split:
                    kinds2[(u, s + 'a')] = kinds.get((u, s), 'TRUE')
        P1 = formula_prio(rows2, luids2, kinds2)
        prio = {t: rows[t.rstrip('ab')]['prio'] + P1[t] - P0[t.rstrip('ab')] for t in luids2}
        for t in luids2:
            op.setdefault(t, op.get(t.rstrip('ab'), ''))
        new = list_schedule(rows2, luids2, kinds2, prio, maxblk, op)
        if new is None:
            if debug: sys.stderr.write('resched %s %s: skip 7\n' % (fn, luids[0]))
            continue
        new = [t for t in new if t in units]
        # dbr: a branch whose slot was filled takes the insn just before it, when that one can go there
        out = []
        ok = True
        for t in new:
            if t in slots:
                if not out or BRANCH.match(op[out[-1]]) or '[length = 4]' not in L[units[out[-1]]] and not re.search(r' # \d+[ab]$', L[units[out[-1]]]) \
                        or regs(args[out[-1]][:1]) & (regs(args[t]) | {'$31'}):
                    ok = False
                    break
                s = out.pop()
                out += ['.set\tnoreorder', '.set\tnomacro', t, s, '.set\tmacro', '.set\treorder']
            else:
                out.append(t)
        if not ok:
            if debug: sys.stderr.write('resched %s %s: skip 8\n' % (fn, luids[0]))
            continue
        body = ['\t' + t if t.startswith('.set') else L[units[t]] for t in out]
        L[lo:hi + 1] = body + [None] * (hi + 1 - lo - len(body))
    return '\n'.join(x for x in L if x is not None)


def check():
    def code(s, *u):
        return [re.sub(r'\t #.*', '', l) for l in peep(s, *u).split('\n') if l.startswith('\t') and not l.startswith('\t.') and not l.startswith('\t#')]
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
    # F: ulw/usw
    w = lambda t: re.findall(r'\.word\t(0x\w+)', peep(t))
    assert w('\tlwl\t$2,3($5)\n') == ['0x88a20003'] and w('\tswr\t$2,4($3)\n') == ['0xb8620004'] and w('\tswl\t$3,7($sp)\n') == ['0xaba30007']
    assert w('\tulw\t$2,0($5)\n') == ['0x88a20003', '0x98a20000'] and w('\tusw\t$2,4($3)\n') == ['0xa8620007', '0xb8620004']
    print('psp_gcc_peep.py: sanity check OK')


if __name__ == '__main__':
    if sys.argv[1:] == ['--check']:
        check()
    elif len(sys.argv) >= 3:
        a = sys.argv[1:]
        dump = None
        if a[0] == '--sched':
            dump, a = open(a[1]).read(), a[2:]
        if os.environ.get('PSP_GCC_INSAND') == '1':
            # cc1 already emits the later revision's `ins` (tools/psp_gcc_insand.c): only rules D and F
            t, dump = unaligned(drop_unused_objects(open(a[0]).read(), a[2:])), None
        else:
            t = peep(open(a[0]).read(), a[2:])
        open(a[1], 'w').write(resched(t, dump) if dump else t)
    else:
        sys.exit(__doc__)
