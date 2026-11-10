#!/usr/bin/env python3
"""proto_conflicts.py [MODULE] [-n N] [--name F]: callees whose local prototypes disagree across the files of a
module (src/<m>/*.c plus include/). Each file of the decomp declares its own prototypes; a parameter typed `int`
in one file and `unsigned short` in another changes argument loads/andi at the call sites (docs/matching.md,
round 14/15). Signatures are normalized (parameter names dropped, `unsigned` = `unsigned int`); K&R `f();` is
listed as `()` and the definition is marked `DEF`. For each signature: how many files, and the files whose @asm
functions call the callee (`asm:` list) — the candidates to re-test with another prototype.
usage: tools/proto_conflicts.py libduel_eng -n 20        sanity: tools/proto_conflicts.py --check
       tools/proto_conflicts.py libduel_eng --unify CALLEE ["void CALLEE(int a);" ...]
--unify tries each signature (plus K&R and the given declarations) in every file that declares CALLEE and reports
matched functions lost and @asm words changed (a few minutes for a callee declared in 50 files)."""
import re, glob, sys, collections

KW = {"int", "short", "char", "long", "unsigned", "signed", "void", "float", "double", "const", "volatile",
      "struct", "union", "enum"}
DECL = re.compile(r"^[ \t]*(?:extern\s+|static\s+)?([A-Za-z_][\w \t\*]*?)\b(\w+)\s*\(([^;{}()]*)\)\s*(;|\n?\s*\{)", re.M)


def norm_type(t):
    t = " ".join(t.replace("*", " * ").split())
    t = re.sub(r"\bsigned (int|short|char)\b", r"\1", t)
    t = re.sub(r"\bunsigned(?! (int|short|char|long))\b", "unsigned int", t)
    t = re.sub(r"\b(short|long) int\b", r"\1", t)
    return t.replace(" *", "*")


def norm_params(p):
    p = p.strip()
    if p == "":
        return "()"
    out = []
    for a in p.split(","):
        toks = a.replace("*", " * ").split()
        if len(toks) > 1 and toks[-1] not in KW and toks[-1] != "*" and toks[-2] not in ("struct", "union", "enum"):
            toks = toks[:-1]  # drop the parameter name
        out.append(norm_type(" ".join(toks)))
    return "(" + ", ".join(out) + ")"


def strip_comments(s):
    s = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), s, flags=re.S)
    return re.sub(r"//[^\n]*", "", s)


def asm_callers(src):
    """names called inside #ifdef NON_MATCHING drafts (functions still @asm) of one file"""
    called = set()
    for m in re.finditer(r"^#ifdef NON_MATCHING[^\n]*\n(.*?)^#endif", src, re.M | re.S):
        called.update(re.findall(r"\b(\w+)\s*\(", m.group(1)))
    return called


def scan(module):
    sigs = collections.defaultdict(lambda: collections.defaultdict(list))  # name -> sig -> [file(DEF)]
    asmcall = collections.defaultdict(set)  # name -> files whose asm drafts call it
    files = sorted(glob.glob(f"src/{module}/*.c")) + sorted(glob.glob("include/*.h"))
    for f in files:
        raw = open(f, errors="replace").read()
        s = strip_comments(raw)
        seen = set()
        for m in DECL.finditer(s):
            ret, name, params, end = m.groups()
            ret = norm_type(ret)
            if not ret or ret.split()[-1] in ("return", "else", "if", "while", "for", "switch", "sizeof") \
                    or name in ("if", "while", "for", "switch", "return", "sizeof"):
                continue
            sig = ret + " " + norm_params(params)
            tag = f + (" DEF" if end != ";" else "")
            if (name, sig, tag) in seen:
                continue
            seen.add((name, sig, tag))
            sigs[name][sig].append(tag)
        if f.endswith(".c"):
            for n in asm_callers(s):
                asmcall[n].add(f)
    return sigs, asmcall


def conflicts(sigs):
    """callees with >1 signature, ignoring K&R `()` (compatible with anything): sorted by files involved"""
    res = []
    for name, d in sigs.items():
        typed = {k for k in d if not k.endswith("()")}
        if len(typed) > 1:
            res.append((sum(len(v) for v in d.values()), name, d))
    return sorted(res, key=lambda x: (-x[0], x[1]))


def unify(module, callee, extra):
    """put each candidate prototype of CALLEE (every signature seen, K&R, EXTRA) in every file of the module that
    declares it locally; print matched functions lost (check_file) and @asm word changes (mwcc_check). A candidate
    with `lost 0` is a prototype all the matching code agrees with: the original header, most likely."""
    import subprocess, tempfile, os
    sigs, _ = scan(module)
    syms = {int(m.group(2), 16): m.group(1) for m in
            re.finditer(r"^(\w+) = (0x[0-9A-Fa-f]+);", open(f"symbols/{module}.txt").read(), re.M)}
    pat = re.compile(r"^[A-Za-z][^\n(]*\b%s\([^;{\n]*\);" % callee, re.M)
    tmp = tempfile.mkdtemp(dir="build")

    def check(path):
        out = subprocess.run(["python3", "tools/check_file.py", module, path], capture_output=True, text=True).stdout
        return set(re.findall(r"^OK\s+(\w+)", out, re.M))

    def words(path, fn, a):
        out = subprocess.run(["python3", "tools/mwcc_check.py", path, fn, module, hex(a), "--flags",
                              "-O4,p -sdatathreshold 0 -i include -DNON_MATCHING"], capture_output=True, text=True).stdout
        m = re.search(r"(\d+) B compiled vs (\d+) B original, (\d+) differing", out)
        return "err" if not m else int(m.group(3)) + (0 if m.group(1) == m.group(2) else 1000)

    files = []
    for f in sorted(glob.glob(f"src/{module}/*.c")):
        s = open(f).read()
        if not pat.search(s):
            continue
        asm = []
        for m in re.finditer(r"^// @asm (0x[0-9A-Fa-f]+)-", s, re.M):
            fn = syms.get(int(m.group(1), 16))
            body = fn and re.search(r"^[A-Za-z][^;\n]*\b%s\([^;]*?\)\s*\n\{(.*?)^\}" % fn, s, re.M | re.S)
            if body and re.search(r"\b%s\s*\(" % callee, body.group(1)):
                asm.append((fn, int(m.group(1), 16)))
        files.append((f, s, asm))

    def variant(f, s, decl):
        path = os.path.join(tmp, os.path.basename(f))
        open(path, "w").write(pat.sub(lambda m: decl, s))
        return path
    base = {}
    for f, s, asm in files:
        path = variant(f, s, pat.search(s).group(0))
        base[f] = (check(path), {fn: words(path, fn, a) for fn, a in asm})
    cands = []
    for sig in sigs[callee]:
        ret, params = sig.split(" (", 1)
        cands.append(f"{ret} {callee}({params[:-1]});")
    ret = next(iter(sigs[callee])).split(" (")[0]
    cands = list(dict.fromkeys(cands + [f"{ret} {callee}();"] + extra))
    for decl in cands:
        lost, asm_d = [], []
        for f, s, asm in files:
            path = variant(f, s, decl)
            lost += sorted(base[f][0] - check(path))
            for fn, a in asm:
                w = words(path, fn, a)
                if w != base[f][1][fn]:
                    asm_d.append(f"{fn} {base[f][1][fn]}->{w}")
        print(f"{decl}\n   lost {len(lost)}: {' '.join(lost)[:300]}\n   asm: {' '.join(asm_d)}", flush=True)
    import shutil
    shutil.rmtree(tmp)


def main():
    args = sys.argv[1:]
    if "--check" in args:
        sigs, asmcall = scan("libduel_eng")
        d = sigs["duel_088C77D4"]
        assert any(t.endswith(" DEF") for t in d["void (unsigned int, unsigned short, unsigned short, unsigned short)"])
        assert norm_params("unsigned p, Card *c, struct S s, int") == "(unsigned int, Card*, struct S, int)"
        assert any(n == "duel_host_call" for _, n, _ in conflicts(sigs))
        print("proto_conflicts.py: ok")
        return
    if "--unify" in args:
        i = args.index("--unify")
        module = args[0] if i else "libduel_eng"
        return unify(module, args[i + 1], args[i + 2:])
    n = int(args[args.index("-n") + 1]) if "-n" in args else 30
    only = args[args.index("--name") + 1] if "--name" in args else None
    module = next((a for a in args if not a.startswith("-") and not a.isdigit() and a != only), "libduel_eng")
    sigs, asmcall = scan(module)
    rows = conflicts(sigs)
    if only:
        rows = [r for r in rows if r[1] == only] or [(0, only, sigs.get(only, {}))]
    for total, name, d in rows[:n]:
        print(f"{name}: {len(d)} signatures, {total} declarations")
        for sig, fs in sorted(d.items(), key=lambda x: -len(x[1])):
            asm = sorted({f.split()[0] for f in fs} & asmcall[name])
            short = [f.replace(f"src/{module}/", "") for f in fs]
            print(f"  {len(fs):3d}x {sig}")
            print(f"       {' '.join(short)}")
            if asm:
                print(f"       asm: {' '.join(a.replace(f'src/{module}/', '') for a in asm)}")


if __name__ == "__main__":
    main()
