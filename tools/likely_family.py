#!/usr/bin/env python3
"""likely_family.py: classify the queue functions (docs/matching_queue.tsv) by the two branch-likely
families of docs/matching.md ("Desvios likely: quem recebe a cópia"). Compiles each file with
-DNON_MATCHING (mwcc 219) and compares with build/elf/<m>.elf.

  likely  = the original has `bXX; nop` where mwcc emits `bXXl` with the copy of the target's first
            instruction (same size). Target kind: `ret` (shared return block, fixable: make an
            unconditional jump the first jump into it) or `loop` (loop header, no form found).
  dead    = dead copies in the original (likely-branch target's first instruction left behind after
            an unconditional jump, nothing else reaches it) minus those in the compiled code.

usage: likely_family.py [MODULE|FUNC ...]   one TSV line per function with a hit:
         module function size diff_words likely(ret/loop) dead_missing
       likely_family.py --check             sanity check (st_cond_daytime matches, dead copy counter)"""
import os, re, struct, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mwcc_check import object_function, BASE, SEG, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16

FLAGS = "-O4,p -sdatathreshold 0 -i include -DNON_MATCHING".split()


def off(w):
    return (w & 0xFFFF) - (0x10000 if w & 0x8000 else 0)


def cond(w):  # conditional branch: (kind, likely) or None
    op, rs, rt = w >> 26, (w >> 21) & 31, (w >> 16) & 31
    if op in (4, 5, 6, 7) and not (op == 4 and rs == 0 and rt == 0):
        return op, False
    if op in (20, 21, 22, 23):
        return op - 16, True
    if op == 1 and rt < 4:
        return 100 + (rt & 1), rt >= 2
    return None


def is_b(w):
    return w >> 26 == 4 and (w >> 16) & 0x3FF == 0


def uncond(w):
    return w >> 26 == 2 or is_b(w) or (w >> 26 == 0 and w & 0x3F == 8)


def targets(o):
    return {i + 1 + off(w): i for i, w in enumerate(o) if cond(w) or is_b(w)}


def dead_copies(o):
    tg, n = {i + 1 + off(w) for i, w in enumerate(o) if cond(w) or is_b(w)}, 0
    for i, w in enumerate(o):
        c = cond(w)
        if c and c[1]:
            t = i + 1 + off(w)
            if 3 <= t < len(o) and i + 1 < len(o) and o[t - 1] == o[i + 1] and uncond(o[t - 3]) and t - 1 not in tg:
                n += 1
    return n


def likely(new, old):  # (shared-return hits, loop-header hits)
    ret = loop = 0
    if len(new) != len(old):
        return ret, loop
    back = {i + 1 + off(w) for i, w in enumerate(old) if (cond(w) or is_b(w)) and off(w) < 0}
    for i in range(len(old) - 1):
        a, b = cond(old[i]), cond(new[i])
        if a and b and not a[1] and b[1] and a[0] == b[0] and old[i + 1] == 0 and off(new[i]) == off(old[i]) + 1:
            t = i + 1 + off(old[i])
            if 0 <= t < len(old) and new[i + 1] == old[t]:
                if t in back or t + 1 in back:  # the latch may branch past the header's 1st insn
                    loop += 1
                else:
                    ret += 1
    return ret, loop


def words(b):
    return [struct.unpack_from("<I", b, i)[0] for i in range(0, len(b) - 3, 4)]


def ndiff(new, rel, old):
    def same(i):
        w, v, t = new[i], old[i], rel.get(4 * i)
        if t == R_MIPS_26: return w >> 26 == v >> 26
        if t in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16): return w >> 16 == v >> 16
        return w == v
    n = min(len(new), len(old))
    return sum(not same(i) for i in range(n)) + abs(len(new) - len(old))


_objs = {}


def compiled(src, fn):
    if src not in _objs:
        obj = f"build/tmp/likely/{os.path.basename(src)[:-2]}.o"
        os.makedirs(os.path.dirname(obj), exist_ok=True)
        r = subprocess.run(["tools/mwcc.sh", *FLAGS, "-c", src, "-o", obj], capture_output=True)
        _objs[src] = obj if r.returncode == 0 else None
    if not _objs[src]:
        return None, None
    try:
        code, rel = object_function(_objs[src], fn)
    except SystemExit:
        return None, None
    return words(code), rel


_mods = {}


def original(m, addr, size):
    if m not in _mods:
        fs = sorted(int(x, 16) for x in re.findall(r"= (0x[0-9A-Fa-f]+); // type:func", open(f"symbols/{m}.txt").read()))
        _mods[m] = fs, open(f"build/elf/{m}.elf", "rb").read()
    fs, d = _mods[m]
    end = next((x for x in fs if x > addr), addr + size)
    return words(d[addr - BASE + SEG:end - BASE + SEG])


def classify(m, fn, addr, src, size=0):
    old = original(m, addr, size)
    new, rel = compiled(src, fn)
    if new is None:
        return None
    r, l = likely(new, old)
    return len(old) * 4, ndiff(new, rel, old), r, l, dead_copies(old) - dead_copies(new)


def check():
    sym = open("symbols/libstory.txt").read()
    a = int(re.search(r"^st_cond_daytime = (0x[0-9A-F]+)", sym, re.M).group(1), 16)
    res = classify("libstory", "st_cond_daytime", a, "src/libstory/s_chr.c")
    assert res and res[1] == 0, res
    # bnezl at,+5; sll (copy); addiu; b back; addiu; sll (dead) -> one dead copy
    o = [0x54200005, 0x00071880, 0x2484FFE0, 0x1000FFFB, 0x24E70001, 0x00071880, 0x10A00008]
    assert dead_copies(o) == 1, dead_copies(o)
    print("likely_family.py: ok")


def main():
    if "--check" in sys.argv:
        return check()
    sel = set(sys.argv[1:])
    rows = [l.rstrip("\n").split("\t") for l in open("docs/matching_queue.tsv")][1:]
    for fz, size, m, fn, addr, src in rows:
        if sel and m not in sel and fn not in sel:
            continue
        res = classify(m, fn, int(addr, 16), src, int(size))
        if res and (res[2] or res[3] or res[4] > 0):
            sz, d, r, l, dead = res
            kind = ",".join(k for k, v in (("ret", r), ("loop", l)) if v) or "-"
            print(f"{m}\t{fn}\t{sz}\t{d}\t{kind}\t{max(dead, 0)}", flush=True)


if __name__ == "__main__":
    main()
