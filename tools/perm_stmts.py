#!/usr/bin/env python3
"""Try every order of N consecutive lines of a C file (statements whose order mwcc turns into register or
stack-slot choices) and keep the first order that makes FUNC match (tools/asm_try.sh reports 0 differing words).
The file is restored when no order matches. Up to 6 lines (720 builds); each build takes ~1 s.
usage: MODULE=<m> perm_stmts.py src/<m>/f.c FUNC FIRST_LINE N      (lines FIRST_LINE..FIRST_LINE+N-1, 1-based)
       perm_stmts.py --check"""
import itertools, os, subprocess, sys


def differing(func):
    out = subprocess.run(["sh", "tools/asm_try.sh", func], capture_output=True, text=True).stdout.strip()
    last = out.splitlines()[-1] if out else ""
    return int(last.split(" differing")[0].split()[-1]) if " differing" in last else 10**6


def run(path, func, first, n):
    src = open(path).read().splitlines(keepends=True)
    block = src[first - 1:first - 1 + n]
    best = (differing(func), tuple(range(n)))
    for p in itertools.permutations(range(n)):
        open(path, "w").write("".join(src[:first - 1] + [block[i] for i in p] + src[first - 1 + n:]))
        d = differing(func)
        if d < best[0]:
            best = (d, p)
            print(f"{d:4d} differing  order {p}", flush=True)
        if d == 0:
            return 0
    open(path, "w").write("".join(src))
    print(f"no order matches; best {best[0]} differing with order {best[1]} (file restored)")
    return 1


if sys.argv[1:] == ["--check"]:  # permuting one line is the identity: the file must come back byte-identical
    p = "src/BOOT/anim.c"; before = open(p).read()
    os.environ["MODULE"] = "BOOT"
    assert run(p, "anim_eval", 1, 1) == 0 and open(p).read() == before
    print("perm_stmts.py: ok"); sys.exit(0)
if len(sys.argv) != 5:
    sys.exit(__doc__)
sys.exit(run(sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])))
