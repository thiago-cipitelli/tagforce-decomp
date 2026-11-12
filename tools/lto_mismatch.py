#!/usr/bin/env python3
"""Classify the -Wlto-type-mismatch warnings of `make host-lto` (port, phase 5).

usage: tools/lto_mismatch.py LOG [-v]     summary per class; -v lists every severe one
       tools/lto_mismatch.py --test       sanity check

gcc reports only the first differing parameter, so both declarations are re-read from the source
(file:line:col of the warning and of its "previously declared here" note) and compared whole.
Severe = breaks the i386 cdecl ABI even in 32 bits: argument count, float in a different position or
float vs integer, struct by value, float return. Everything else (pointer vs pointer, int vs pointer,
signedness, short vs int, void vs int return, variables) is benign in -m32.
"""
import os, re, sys, glob

LOC = re.compile(r"^(\S+?):(\d+):(\d+): (warning|note): (.*)$")
INTS = set("char short int long unsigned signed enum _Bool size_t".split())


def typedefs(root="."):
    """typedef name -> 'S' (struct/union) or 'F' (float/double)."""
    td = {}
    for f in glob.glob(root + "/include/*.h") + glob.glob(root + "/src/*/*.[ch]"):
        s = open(f, errors="replace").read()
        for m in re.finditer(r"typedef\s+(struct|union)\b[^;{]*(\{)?", s):
            if m.group(2):  # typedef struct {...} Name;
                i, d = m.end(), 1
                while i < len(s) and d:
                    d += {"{": 1, "}": -1}.get(s[i], 0); i += 1
                n = re.match(r"\s*(\w+)", s[i:])
            else:
                n = re.match(r"typedef\s+(?:struct|union)\s+\w+\s+(\w+)\s*;", s[m.start():])
            if n: td[n.group(1)] = "S"
        for m in re.finditer(r"typedef\s+(?:float|double)\s+(\w+)\s*;", s):
            td[m.group(1)] = "F"
    return td


def shape(t, td):
    """one parameter/return type text -> I (integer), P, F, V, S:name, or ... ."""
    t = re.sub(r"\b(const|volatile|register|extern|static|inline|__inline)\b", " ", t).strip()
    if t == "...": return "..."
    if "*" in t or "[" in t or "(" in t: return "P"
    w = t.split()
    if len(w) > 1 and w[-1] not in INTS | {"float", "double", "void"} and not (len(w) == 2 and w[0] in ("struct", "union")):
        w = w[:-1]  # drop the parameter name
    if w[:1] == ["struct"] or w[:1] == ["union"]: return "S:" + w[-1]
    if w in (["float"], ["double"]) or td.get(w[-1]) == "F": return "F"
    if w == ["void"]: return "V"
    if td.get(w[-1]) == "S": return "S:" + w[-1]
    return "I"


def split_params(s):
    out, d, cur = [], 0, ""
    for c in s:
        if c == "," and d == 0: out.append(cur); cur = ""; continue
        d += c in "(["; d -= c in ")]"; cur += c
    out.append(cur)
    return [x.strip() for x in out]


def decl(path, line, col, name, td, cache={}):
    """(return shape, [param shapes] or None for K&R, text) of the declaration of `name` at path:line."""
    if path not in cache:
        try: cache[path] = open(path, errors="replace").read().split("\n")
        except OSError: cache[path] = []
    L = cache[path]
    if line > len(L): return None
    text = " ".join(L[line - 1:line + 15])
    base = text[max(col - 1, 0):]
    m = re.match(re.escape(name) + r"\s*\(", base)
    if not m:
        return None  # variable
    i, d = m.end(), 1
    j = i
    while j < len(base) and d:
        d += {"(": 1, ")": -1}.get(base[j], 0); j += 1
    params = base[i:j - 1].strip()
    ret = L[line - 1][:max(col - 1, 0)].strip() or (L[line - 2].strip() if line > 1 else "int")
    ret = re.sub(r"^.*[;{}]", "", ret).strip() or "int"
    rs = "P" if ret.endswith("*") or name in ret and "*" in ret else shape(ret, td)
    if params == "": ps = None  # K&R
    elif params == "void": ps = []
    else: ps = [shape(p, td) for p in split_params(params)]
    return rs, ps, (ret + " " + name + "(" + params + ")")


def classify(a, b):
    """severity class of two declarations (ret, params)."""
    (ra, pa), (rb, pb) = a, b
    if pa is None or pb is None:  # K&R: floats/structs passed promoted or not at all
        p = pa if pb is None else pb
        if p and any(x == "F" or x.startswith("S:") for x in p): return "float ou struct via K&R"
        p = []
        pa, pb = pa or [], pb or []
        if not pa or not pb: pa = pb = []
    va, vb = "..." in pa, "..." in pb
    fa, fb = [x for x in pa if x != "..."], [x for x in pb if x != "..."]
    n = min(len(fa), len(fb))
    if len(fa) != len(fb) and not (va and len(fb) >= len(fa) or vb and len(fa) >= len(fb)):
        return "número de argumentos"
    for x, y in zip(fa[:n], fb[:n]):
        if (x == "F") != (y == "F"): return "float vs inteiro (posição)"
    for x, y in zip(fa[:n], fb[:n]):
        if (x.startswith("S:") or y.startswith("S:")) and x != y: return "struct por valor"
    if (ra == "F") != (rb == "F"): return "retorno float"
    if (ra.startswith("S:") or rb.startswith("S:")) and ra != rb: return "struct por valor"
    if ra != rb and "V" in (ra, rb): return "benigno: retorno void"
    return "benigno: tipos"


SEVERE = ("número de argumentos", "float vs inteiro (posição)", "struct por valor", "retorno float", "float ou struct via K&R")


def parse(log):
    """[(name, (fileA,line,col), (fileB,line,col))] for every warning."""
    out, cur = [], None
    for ln in log.split("\n"):
        m = LOC.match(ln)
        if not m: continue
        f, l, c, kind, msg = m.group(1), int(m.group(2)), int(m.group(3)), m.group(4), m.group(5)
        if kind == "warning":
            n = re.search(r"type of ‘(\w+)’", msg)
            if n:
                cur = [n.group(1), (f, l, c), None]; out.append(cur)
        elif cur and cur[2] is None:  # first note = the other declaration
            cur[2] = (f, l, c)
    return out


def main(argv):
    if argv[1:2] == ["--test"]:
        return test()
    log = open(argv[1], errors="replace").read()
    td = typedefs()
    counts, sev = {}, []
    for name, a, b in parse(log):
        da = decl(*a, name, td); db = decl(*b, name, td) if b else None
        if da is None or db is None:
            c = "benigno: variável" if da is None else "benigno: tipos"
        else:
            c = classify(da[:2], db[:2])
        counts[c] = counts.get(c, 0) + 1
        if c in SEVERE: sev.append((c, name, a, b, da, db))
    for c in sorted(counts, key=lambda c: (c not in SEVERE, c)):
        print("%5d  %s" % (counts[c], c))
    nsev = sum(counts.get(c, 0) for c in SEVERE)
    print("%5d  total, %d severe, %d symbols defined in more than one module/file" % (
        sum(counts.values()), nsev, len(set(re.findall(r"multiple definition of `(\w+)'", log)))))
    if "-v" in argv:
        for c, name, a, b, da, db in sorted(sev):
            print("%s | %s:%d %s | %s:%d %s" % (c, a[0], a[1], da[2], b[0], b[1], db[2]))
    return 0


def test():
    td = {"Vec": "S", "real": "F"}
    assert shape("float x", td) == "F" and shape("unsigned short", td) == "I" and shape("Vec v", td) == "S:Vec"
    assert shape("Vec *v", td) == "P" and shape("real", td) == "F" and shape("int", td) == "I" and shape("struct Foo f", td) == "S:Foo"
    assert shape("void", td) == "V" and shape("void (*cb)(int)", td) == "P" and shape("const char *s", td) == "P"
    assert classify(("V", ["F", "F", "P"]), ("V", ["P", "F", "F"])) == "float vs inteiro (posição)"
    assert classify(("V", ["I"]), ("V", ["I", "I"])) == "número de argumentos"
    assert classify(("V", ["I", "..."]), ("V", ["I", "I"])) == "benigno: tipos"
    assert classify(("F", []), ("I", [])) == "retorno float"
    assert classify(("V", ["S:Vec"]), ("V", ["I"])) == "struct por valor"
    assert classify(("V", None), ("V", ["F"])) == "float ou struct via K&R"
    assert classify(("V", ["I"]), ("I", ["P"])) == "benigno: retorno void"
    log = "a.c:3:6: warning: type of ‘f’ does not match original declaration [-Wlto-type-mismatch]\n" \
          "b.c:9:6: note: type mismatch in parameter 1\nb.c:9:6: note: ‘f’ was previously declared here\n"
    assert parse(log) == [["f", ("a.c", 3, 6), ("b.c", 9, 6)]]
    print("ok")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
