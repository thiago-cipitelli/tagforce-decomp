#!/usr/bin/env python3
"""Random source permuter for one function (docs/matching.md, "Rodada 39 (rB)").

Applies semantics-preserving rewrites to the C of FUNC (the draft under NON_MATCHING), compiles the whole file
with mwccpsp and scores the function by differing words against the original (relocated operands masked, as in
mwcc_check.py; size differences count). A pool of the best variants is kept; each candidate takes a parent from the
pool and applies 1-3 random mutations. Mutations (perm_decls/perm_stmts already cover plain reorderings):
  commute (+ * & | ^ == != and < > <= >= flipped), a += b <-> a = a + b, x * 2^k <-> x << k, x << 1 / x * 2 -> x + x,
  !x <-> x == 0, if (x) <-> if (x != 0), temporary for a subexpression ({ __typeof__(e) t = e; ... }), substitute a
  single-use initialised local, redundant cast of a scalar local/parameter to its own type, a[i] <-> *(a + i),
  p->f <-> (*p).f, &a[i] -> a + i, for <-> while, while -> if/do-while, split/merge declaration and initialiser,
  swap adjacent declarations, late copy of a parameter with a redundant cast (round 35), if (a && b) -> nested if,
  if/else swapped with the condition negated, x++ / ++x / x += 1 / x = x + 1.
The search never edits the source: best variants go to build/tmp/permute/<FUNC>/ (score_N.c = the function text,
log.txt); porting one back and checking its logic is manual. __typeof__ in a variant must become a real type.

usage: MODULE=<m> permute.py src/<m>/f.c FUNC [--minutes 30] [-j 4] [--seed N] [--start score_N.c]
       permute.py --selftest
"""
import argparse, os, random, re, struct, subprocess, sys, threading, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mwcc_check import object_function, BASE, SEG, R_MIPS_26, R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16

# ---------------------------------------------------------------- tokens
TOK = re.compile(r"""(?P<ws>(?:\s|/\*.*?\*/|//[^\n]*|^\#[^\n]*)+)|(?P<t>"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|
    0[xX][0-9a-fA-F]+[uUlL]*|\d+\.\d*(?:[eE][-+]?\d+)?[fFlL]?|\.\d+(?:[eE][-+]?\d+)?[fFlL]?|\d+[uUlLfF]*|[A-Za-z_]\w*|
    \.\.\.|<<=|>>=|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^]=|.)""", re.S | re.M | re.X)
KW = set("""auto break case char const continue default do double else enum extern float for goto if int long register
return short signed sizeof static struct switch typedef union unsigned void volatile while inline __inline __typeof__""".split())
TYPEKW = set("char const double enum float int long short signed struct union unsigned void volatile register static __typeof__".split())
ASSIGN = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}
PREC = {"*": 13, "/": 13, "%": 13, "+": 12, "-": 12, "<<": 11, ">>": 11, "<": 10, ">": 10, "<=": 10, ">=": 10,
        "==": 9, "!=": 9, "&": 8, "^": 7, "|": 6, "&&": 5, "||": 4}
CTRL = {"if", "while", "for", "switch"}


def lex(s):
    """-> list of [pre, text]: each significant token with the whitespace/comments before it; last item has text ''."""
    out, pre = [], ""
    for m in TOK.finditer(s):
        if m.group("ws") is not None:
            pre += m.group("ws")
        else:
            out.append([pre, m.group("t")]); pre = ""
    out.append([pre, ""])
    return out


def render(toks):
    return "".join(p + t for p, t in toks)


def T(texts):  # new tokens, one space before each
    return [[" ", t] for t in texts]


def isid(t):
    return bool(re.match(r"[A-Za-z_]\w*$", t)) and t not in KW


def isnum(t):
    return bool(re.match(r"\.?\d", t))


class Ctx:
    def __init__(self, types=(), params=None):
        self.types = set(types)   # typedef names
        self.params = params or {}  # name -> type text


def match_fwd(toks, i):
    """index of the bracket closing toks[i]"""
    o = toks[i][1]; c = {"(": ")", "[": "]", "{": "}"}[o]; d = 0
    for j in range(i, len(toks)):
        if toks[j][1] == o: d += 1
        elif toks[j][1] == c:
            d -= 1
            if d == 0: return j
    return None


def match_back(toks, i):
    c = toks[i][1]; o = {")": "(", "]": "[", "}": "{"}[c]; d = 0
    for j in range(i, -1, -1):
        if toks[j][1] == c: d += 1
        elif toks[j][1] == o:
            d -= 1
            if d == 0: return j
    return None


def is_type_start(t, ctx):
    return t in TYPEKW or t in ctx.types


def is_cast(toks, i, ctx):
    """toks[i] == '(' : does the group hold a type name?"""
    j = match_fwd(toks, i)
    if j is None or j == i + 1 or not is_type_start(toks[i + 1][1], ctx): return False
    return all(isid(t) or t in ("*", "const", "volatile", "unsigned", "signed", "int", "char", "short", "long", "struct",
                                "union", "void", "float", "double") for _, t in toks[i + 1:j])


def operand_end(toks, i, ctx):
    """does toks[i] end an operand (so a following * & + - is binary)?"""
    t = toks[i][1]
    if (isid(t) and t not in ctx.types) or isnum(t) or t[:1] in "\"'" or t == "]": return True
    if t in ("++", "--"): return i > 0 and operand_end(toks, i - 1, ctx)
    if t == ")":
        o = match_back(toks, i)
        if o is None: return False
        if o > 0 and toks[o - 1][1] in CTRL: return False
        return not is_cast(toks, o, ctx)
    return False


def is_binary(toks, i, ctx):
    t = toks[i][1]
    if t not in PREC: return False
    if t in ("*", "&", "+", "-"): return i > 0 and operand_end(toks, i - 1, ctx)
    return True


def unary_end(toks, i, ctx):
    """parse a unary expression starting at i; return index after it (or None)"""
    t = toks[i][1]
    if t in ("!", "~", "-", "+", "*", "&", "++", "--"): return unary_end(toks, i + 1, ctx)
    if t == "sizeof":
        if toks[i + 1][1] == "(" and is_cast(toks, i + 1, ctx): return match_fwd(toks, i + 1) + 1
        return unary_end(toks, i + 1, ctx)
    if t == "(":
        j = match_fwd(toks, i)
        if j is None: return None
        if is_cast(toks, i, ctx): return unary_end(toks, j + 1, ctx)
        k = j + 1
    elif isid(t) or isnum(t) or t[:1] in "\"'":
        k = i + 1
    else:
        return None
    while True:  # postfix
        t = toks[k][1]
        if t in ("(", "["): k = match_fwd(toks, k) + 1
        elif t in (".", "->"): k += 2
        elif t in ("++", "--"): k += 1
        else: return k


def right_operand(toks, i, p, ctx):
    """operand right of the binary operator at i (precedence p): [i+1, end)"""
    k = unary_end(toks, i + 1, ctx)
    while k is not None and toks[k][1] in PREC and PREC[toks[k][1]] > p:
        k = unary_end(toks, k + 1, ctx)
    return k


def left_operand(toks, i, p, ctx):
    """start index of the operand left of the binary operator at i"""
    j = i - 1
    while j >= 0:
        t = toks[j][1]
        if t in (")", "]"):
            o = match_back(toks, j)
            if o is None or (t == ")" and o > 0 and toks[o - 1][1] in CTRL): return None
            j = o - 1; continue
        if isid(t) or isnum(t) or t[:1] in "\"'" or t in (".", "->", "++", "--", "sizeof"):
            j -= 1; continue
        if t in ("!", "~"): j -= 1; continue
        if t in PREC:
            if is_binary(toks, j, ctx):
                if PREC[t] < p: break
                j -= 1; continue
            j -= 1; continue  # unary * & + -
        break
    s = j + 1
    return s if s < i else None


def has_top_binop(toks, a, b, ctx):
    d = 0
    for k in range(a, b):
        t = toks[k][1]
        if t in "([": d += 1
        elif t in ")]": d -= 1
        elif d == 0 and (t in ("?", ":") or (t in PREC and is_binary(toks, k, ctx))): return True
    return False


def paren(toks, a, b, ctx):
    seg = [list(x) for x in toks[a:b]]
    if has_top_binop(toks, a, b, ctx) or (seg and seg[0][1] == "(" and is_cast(toks, a, ctx)):
        return T(["("]) + seg + T([")"])
    return seg


def side_effects(toks, a, b):
    for k in range(a, b):
        t = toks[k][1]
        if t in ASSIGN or t in ("++", "--"): return True
        if t == "(" and k > 0 and (isid(toks[k - 1][1]) or toks[k - 1][1] in (")", "]")): return True
    return False


def stmt_bounds(toks, i):
    """[start, end] of the simple statement holding token i (end = its ';'), None inside control heads"""
    s = i
    while s > 0 and toks[s - 1][1] not in (";", "{", "}"):
        s -= 1
        if toks[s][1] == ")":
            o = match_back(toks, s)
            if o is not None and o > 0 and toks[o - 1][1] in CTRL: s += 1; break
        if toks[s][1] in ("(", "[") and match_fwd(toks, s) and match_fwd(toks, s) >= i:
            if s > 0 and toks[s - 1][1] in CTRL: return None
    if toks[s][1] in ("else", "do", "case", "default"): return None
    e = i
    while toks[e][1] not in (";", "") and toks[e][1] != "{":
        if toks[e][1] in "([": e = match_fwd(toks, e)
        e += 1
    return (s, e) if toks[e][1] == ";" else None


# ---------------------------------------------------------------- declarations
def decls_in(toks, ctx):
    """declarations at the start of every block: list of dicts (start, end(';'), type, name, init(a,b) or None)"""
    res = []
    for b in [i for i, x in enumerate(toks) if x[1] == "{"]:
        k = b + 1
        while is_type_start(toks[k][1], ctx) and toks[k + 1][1] != "(":
            e = k
            while toks[e][1] not in (";", ""):
                if toks[e][1] in "([{": e = match_fwd(toks, e)
                e += 1
            if toks[e][1] != ";": break
            d = parse_decl(toks, k, e, ctx)
            d["block"] = b
            res.append(d)
            k = e + 1
    return res


def parse_decl(toks, s, e, ctx):
    k = s
    while is_type_start(toks[k][1], ctx) or (toks[k - 1][1] in ("struct", "union", "enum")):
        k += 1
    base = [t for _, t in toks[s:k]]
    stars = []
    while toks[k][1] == "*": stars.append("*"); k += 1
    d = {"start": s, "end": e, "base": base, "type": " ".join(base + stars), "name": None, "init": None, "multi": False}
    if not isid(toks[k][1]): return d
    d["name"] = toks[k][1]; k += 1
    arr = toks[k][1] == "["
    if arr: k = match_fwd(toks, k) + 1
    dd = 0
    for j in range(k, e):
        if toks[j][1] in "([{": dd += 1
        elif toks[j][1] in ")]}": dd -= 1
        elif dd == 0 and toks[j][1] == ",": d["multi"] = True
    if arr or d["multi"]: d["type"] = None
    if toks[k][1] == "=": d["init"] = (k + 1, e)
    d["eq"] = k
    return d


def scalar_types(toks, ctx):
    """name -> type for scalar/pointer locals and parameters"""
    ty = {n: t for n, t in ctx.params.items() if t}
    for d in decls_in(toks, ctx):
        if d["name"] and d["type"] and "static" not in d["base"] and (d["type"].endswith("*") or set(d["base"]) <= TYPEKW):
            ty[d["name"]] = d["type"].replace("register ", "").replace("volatile ", "")
    return ty


def uses(toks, name, a=0, b=None):
    b = len(toks) if b is None else b
    return [k for k in range(a, b) if toks[k][1] == name and toks[k - 1][1] not in (".", "->")]


# ---------------------------------------------------------------- mutations: toks, ctx, rng -> toks or None
def m_commute(toks, ctx, r):
    ops = [i for i, x in enumerate(toks) if x[1] in ("+", "*", "&", "|", "^", "==", "!=", "<", ">", "<=", ">=")
           and is_binary(toks, i, ctx)]
    if not ops: return None
    i = r.choice(ops); op = toks[i][1]; p = PREC[op]
    a = left_operand(toks, i, p, ctx); b = right_operand(toks, i, p, ctx)
    if a is None or b is None: return None
    nop = {"<": ">", ">": "<", "<=": ">=", ">=": "<="}.get(op, op)
    L = paren(toks, a, i, ctx); R = paren(toks, i + 1, b, ctx)
    R[0][0] = toks[a][0]; L[0][0] = " "
    return toks[:a] + R + [[toks[i][0], nop]] + L + toks[b:]


def m_compound(toks, ctx, r):
    cands = []
    for i, x in enumerate(toks):
        if x[1] in ASSIGN and x[1] != "=":
            sb = stmt_bounds(toks, i)
            if sb and not side_effects(toks, sb[0], i): cands.append(("split", i, sb))
        elif x[1] == "=":
            sb = stmt_bounds(toks, i)
            if sb and sb[0] < i:
                n = i - sb[0]; lv = [t for _, t in toks[sb[0]:i]]
                if [t for _, t in toks[i + 1:i + 1 + n]] == lv and toks[i + 1 + n][1] in PREC and not side_effects(toks, sb[0], i):
                    cands.append(("join", i, sb))
    if not cands: return None
    kind, i, (s, e) = r.choice(cands)
    if kind == "split":
        op = toks[i][1][:-1]
        lv = [list(x) for x in toks[s:i]]; lv[0][0] = " "
        return toks[:i] + [[toks[i][0], "="]] + lv + T([op]) + paren(toks, i + 1, e, ctx) + toks[e:]
    n = i - s; o = i + 1 + n; op = toks[o][1]
    if op in ("&&", "||", "<", ">", "<=", ">=", "==", "!="): return None
    d = 0  # E must hold no top-level operator binding as loose as op
    for k in range(o + 1, e):
        t = toks[k][1]
        if t in "([": d += 1
        elif t in ")]": d -= 1
        elif d == 0 and t in PREC and is_binary(toks, k, ctx) and PREC[t] <= PREC[op]: return None
    return toks[:i] + [[toks[i][0], op + "="]] + toks[o + 1:]


def m_shift(toks, ctx, r):
    c = []
    for i, x in enumerate(toks):
        if x[1] in ("*", "<<") and is_binary(toks, i, ctx) and re.match(r"(0x[0-9a-fA-F]+|\d+)[uU]?$", toks[i + 1][1]) \
                and toks[i + 2][1] not in ("(", "[", ".", "->"):
            v = int(re.sub(r"[uU]$", "", toks[i + 1][1]), 0)
            if x[1] == "*" and v > 1 and v & (v - 1) == 0: c.append((i, "<<", str(v.bit_length() - 1)))
            if x[1] == "<<" and 0 < v < 31: c.append((i, "*", str(1 << v)))
            if v in (1, 2) and isid(toks[i - 1][1]) and toks[i - 2][1] not in (".", "->", "*", "&") and (x[1] == "<<") == (v == 1):
                c.append((i, "+", toks[i - 1][1]))
    if not c: return None
    i, op, n = r.choice(c)
    p = PREC[toks[i][1]]
    if op in ("<<", "*") and toks[i + 2][1] in PREC and PREC[toks[i + 2][1]] > PREC[op]: return None
    a = left_operand(toks, i, p, ctx)
    if a is None: return None
    if op == "+" and a != i - 1: return None
    out = toks[:a] + T(["("]) + toks[a:i] + [[toks[i][0], op], [toks[i + 1][0], n]] + T([")"]) + toks[i + 2:]
    out[a][0] = toks[a][0]
    return out


def m_not(toks, ctx, r):
    c = []
    for i, x in enumerate(toks):
        if x[1] == "!" and toks[i + 1][1] != "!": c.append(("not", i))
        if x[1] == "==" and toks[i + 1][1] == "0" and is_binary(toks, i, ctx) and toks[i + 2][1] in (")", ";", "&&", "||", ","):
            c.append(("eq", i))
        if x[1] == "!=" and toks[i + 1][1] == "0" and toks[i + 2][1] == ")" and is_binary(toks, i, ctx):
            o = match_back(toks, i + 2)
            if o and toks[o - 1][1] in ("if", "while") and left_operand(toks, i, PREC["!="], ctx) == o + 1: c.append(("ne", i))
        if x[1] in ("if", "while") and toks[i + 1][1] == "(":
            j = match_fwd(toks, i + 1)
            if not has_top_binop(toks, i + 2, j, ctx) or toks[j - 2][1] != "!=": c.append(("cond", i))
    if not c: return None
    k, i = r.choice(c)
    if k == "not":
        e = unary_end(toks, i + 1, ctx)
        if e is None: return None
        return toks[:i] + [[toks[i][0], "("]] + paren(toks, i + 1, e, ctx) + T(["==", "0", ")"]) + toks[e:]
    if k == "eq":
        a = left_operand(toks, i, PREC["=="], ctx)
        if a is None: return None
        seg = T(["!", "("]) + toks[a:i] + T([")"]); seg[0][0] = toks[a][0]
        return toks[:a] + seg + toks[i + 2:]
    if k == "ne":
        return toks[:i] + toks[i + 2:]
    j = match_fwd(toks, i + 1)
    return toks[:i + 2] + paren(toks, i + 2, j, ctx) + T(["!=", "0"]) + toks[j:]


_tmp = [0]


def m_temp(toks, ctx, r):
    """{ __typeof__(e) t = e; S[e -> t] } for a side-effect-free subexpression e of a simple statement S"""
    c = []
    for i, x in enumerate(toks):
        if x[1] in PREC and is_binary(toks, i, ctx):
            a = left_operand(toks, i, PREC[x[1]], ctx); b = right_operand(toks, i, PREC[x[1]], ctx)
            if a is not None and b is not None: c += [(a, i), (i + 1, b), (a, b)]
        if x[1] == "(" and i > 0 and isid(toks[i - 1][1]) and toks[i - 1][1] not in CTRL:
            c.append((i - 1, match_fwd(toks, i) + 1))
    r.shuffle(c)
    for a, b in c[:8]:
        if b - a < 2 and not isid(toks[a][1]): continue
        sb = stmt_bounds(toks, a)
        if not sb or toks[sb[0]][1] in ("break", "continue", "goto") or is_type_start(toks[sb[0]][1], ctx): continue
        s, e = sb
        calls = side_effects(toks, a, b)
        if calls and (any(toks[k][1] in ASSIGN or toks[k][1] in ("++", "--") for k in range(a, b))
                      or side_effects(toks, s, a)): continue
        if not calls and any(toks[k][1] in ("++", "--") or (toks[k][1] in ASSIGN and toks[k][1] != "=") for k in range(s, a)):
            continue
        _tmp[0] += 1; n = "_pt%d" % _tmp[0]
        e_ = [list(y) for y in toks[a:b]]; e_[0][0] = " "
        blk = T(["{", "__typeof__", "("]) + e_ + T([")", n, "="]) + [list(y) for y in e_] + T([";"])
        blk[0][0] = toks[s][0]
        body = [list(y) for y in toks[s:a]] + T([n]) + toks[b:e + 1]
        body[0][0] = " "
        return toks[:s] + blk + body + T(["}"]) + toks[e + 1:]
    return None


def m_untemp(toks, ctx, r):
    ds = [d for d in decls_in(toks, ctx) if d["name"] and d["init"] and d["type"] and "static" not in d["base"]]
    r.shuffle(ds)
    for d in ds:
        a, b = d["init"]
        if side_effects(toks, a, b): continue
        u = uses(toks, d["name"], d["end"])
        if len(u) != 1: continue
        k = u[0]
        if toks[k + 1][1] in ASSIGN or toks[k + 1][1] in ("++", "--", "[") or toks[k - 1][1] in ("&", "++", "--"): continue
        ids = {t for _, t in toks[a:b] if isid(t)}
        if any(toks[j][1] in ids and (toks[j + 1][1] in ASSIGN or toks[j + 1][1] in ("++", "--") or toks[j - 1][1] in ("++", "--", "&"))
               for j in range(d["end"], k)): continue
        if any(toks[j][1] == "(" and isid(toks[j - 1][1]) for j in range(d["end"], k)) and \
                any(toks[j][1] in ("*", "->", "[") for j in range(a, b)): continue  # a call could change memory
        ty = d["type"]
        e_ = T(["(", "(", ty, ")", "("]) + [list(y) for y in toks[a:b]] + T([")", ")"])
        e_[0][0] = toks[k][0]
        out = toks[:k] + e_ + toks[k + 1:]
        return out[:d["start"]] + out[d["end"] + 1:]
    return None


def m_cast(toks, ctx, r):
    ty = scalar_types(toks, ctx)
    c = [i for i, x in enumerate(toks) if x[1] in ty and toks[i - 1][1] not in (".", "->", "&", "++", "--", "*")
         and toks[i + 1][1] not in ASSIGN and toks[i + 1][1] not in ("++", "--", "[", "(", ",", ";") and not
         (is_type_start(toks[i - 1][1], ctx) or toks[i - 1][1] == "*" and is_type_start(toks[i - 2][1], ctx))]
    c = [i for i in c if not any(d["start"] <= i < d.get("eq", d["start"]) + 1 for d in decls_in(toks, ctx))]
    if not c: return None
    i = r.choice(c)
    return toks[:i] + [[toks[i][0], "("]] + T(["(", ty[toks[i][1]], ")", toks[i][1], ")"]) + toks[i + 1:]


def postfix_start(toks, i, ctx):
    """start of the postfix expression ending just before index i"""
    j = i - 1
    while j >= 0:
        t = toks[j][1]
        if t in (")", "]"):
            o = match_back(toks, j)
            if t == ")" and (o == 0 or not (isid(toks[o - 1][1]) or toks[o - 1][1] in (")", "]"))):
                if is_cast(toks, o, ctx) or (o > 0 and toks[o - 1][1] in CTRL): return None
                return o  # parenthesised primary
            j = o - 1; continue
        if isid(t):
            if j > 0 and toks[j - 1][1] in (".", "->"): j -= 2; continue
            return j
        return None
    return None


def m_index(toks, ctx, r):
    c = []
    for i, x in enumerate(toks):
        if x[1] == "[" and i > 0 and (isid(toks[i - 1][1]) or toks[i - 1][1] in (")", "]")) and not is_type_start(toks[i - 2][1], ctx):
            c.append(("idx", i))
        if x[1] == "->": c.append(("arrow", i))
        if x[1] == "*" and toks[i + 1][1] == "(" and not is_binary(toks, i, ctx): c.append(("deref", i))
    if not c: return None
    k, i = r.choice(c)
    if k == "idx":
        a = postfix_start(toks, i, ctx)
        if a is None or any(d["start"] <= i <= d["end"] and "=" not in [t for _, t in toks[d["start"]:i]] for d in decls_in(toks, ctx)):
            return None
        j = match_fwd(toks, i)
        base = [list(y) for y in toks[a:i]]; base[0][0] = " "
        if a > 0 and toks[a - 1][1] == "&" and not is_binary(toks, a - 1, ctx):
            seg = T(["("]) + base + T(["+"]) + paren(toks, i + 1, j, ctx) + T([")"])
            seg[0][0] = toks[a - 1][0]
            return toks[:a - 1] + seg + toks[j + 1:]
        seg = T(["(", "*", "("]) + base + T(["+"]) + paren(toks, i + 1, j, ctx) + T([")", ")"])
        seg[0][0] = toks[a][0]
        return toks[:a] + seg + toks[j + 1:]
    if k == "arrow":
        a = postfix_start(toks, i, ctx)
        if a is None: return None
        base = [list(y) for y in toks[a:i]]; base[0][0] = " "
        seg = T(["(", "*"]) + base + T([")", "."])
        seg[0][0] = toks[a][0]
        return toks[:a] + seg + toks[i + 1:]
    j = match_fwd(toks, i + 1)  # *(A + I) -> A[I]
    d, plus = 0, None
    for q in range(i + 2, j):
        t = toks[q][1]
        if t in "([": d += 1
        elif t in ")]": d -= 1
        elif d == 0 and t in PREC and is_binary(toks, q, ctx):
            if t != "+" or plus is not None: return None
            plus = q
    if plus is None or postfix_start(toks, plus, ctx) != i + 2: return None
    seg = [list(y) for y in toks[i + 2:plus]] + T(["["]) + toks[plus + 1:j] + T(["]"])
    seg[0][0] = toks[i][0]
    if toks[j + 1][1] in ("[", ".", "->", "(", "++", "--"): seg = T(["("]) + seg + T([")"])
    return toks[:i] + seg + toks[j + 1:]


def body_of(toks, k):
    """statement starting at k -> index of its last token"""
    t = toks[k][1]
    if t == "{": return match_fwd(toks, k)
    if t in ("if", "while", "for", "switch"):
        e = body_of(toks, match_fwd(toks, k + 1) + 1)
        if t == "if" and toks[e + 1][1] == "else": e = body_of(toks, e + 2)
        return e
    if t == "do":
        e = body_of(toks, k + 1)
        return match_fwd(toks, e + 2) + 1
    while toks[k][1] != ";":
        if toks[k][1] in "([{": k = match_fwd(toks, k)
        k += 1
    return k


def m_loop(toks, ctx, r):
    c = [i for i, x in enumerate(toks) if x[1] in ("for", "while") and toks[i - 1][1] != "}" or
         x[1] == "while" and not (toks[i - 1][1] == "}" and toks[match_back(toks, i - 1) - 1][1] == "do")]
    c = [i for i in c if toks[i][1] == "for" or toks[i - 1][1] != "}" or toks[match_back(toks, i - 1) - 1][1] != "do"]
    if not c: return None
    i = r.choice(c)
    h = match_fwd(toks, i + 1); b0 = h + 1; b1 = body_of(toks, b0)
    if any(t == "continue" for _, t in toks[b0:b1 + 1]): return None
    body = [list(y) for y in toks[b0:b1 + 1]]
    if body[0][1] != "{": body = T(["{"]) + body + T(["}"])
    if toks[i][1] == "while":
        cond = toks[i + 2:h]
        if r.random() < 0.5:
            return toks[:i] + [[toks[i][0], "for"]] + T(["(", ";"]) + cond + T([";", ")"]) + toks[h + 1:]
        if side_effects(toks, i + 2, h): return None
        cd = [list(y) for y in cond]
        out = [[toks[i][0], "if"]] + T(["("]) + cd + T([")", "do"]) + body + T(["while", "("]) + [list(y) for y in cond] + T([")", ";"])
        return toks[:i] + out + toks[b1 + 1:]
    s1 = s2 = None
    d = 0
    for q in range(i + 2, h):
        t = toks[q][1]
        if t in "([": d += 1
        elif t in ")]": d -= 1
        elif d == 0 and t == ";": s1, s2 = (q, s2) if s1 is None else (s1, q)
    init, cond, step = toks[i + 2:s1], toks[s1 + 1:s2], toks[s2 + 1:h]
    cond = [list(y) for y in cond] or T(["1"])
    nb = body[:-1] + ([list(y) for y in step] + T([";"]) if step else []) + [body[-1]]
    out = T(["{"]) + ([list(y) for y in init] + T([";"]) if init else []) + T(["while", "("]) + cond + T([")"]) + nb + T(["}"])
    out[0][0] = toks[i][0]
    return toks[:i] + out + toks[b1 + 1:]


def m_decl(toks, ctx, r):
    ds = decls_in(toks, ctx)
    if not ds: return None
    k = r.randrange(4)
    if k == 0:  # split T v = e;  ->  T v; ... v = e; after the block's declarations
        c = [d for d in ds if d["name"] and d["init"] and not d["multi"] and "static" not in d["base"] and "const" not in d["base"]
             and toks[d["init"][0]][1] != "{"]
        if not c: return None
        d = r.choice(c); a, b = d["init"]
        later = [x for x in ds if x["block"] == d["block"] and x["start"] > d["start"]]
        if any(uses(toks, d["name"], x["start"], x["end"]) for x in later): return None
        if side_effects(toks, a, b) and any(x["init"] and side_effects(toks, *x["init"]) for x in later): return None
        last = max([x["end"] for x in later] + [d["end"]])
        st = T([d["name"], "="]) + [list(y) for y in toks[a:b]] + T([";"])
        st[0][0] = "\n    "
        return toks[:d["eq"]] + toks[b:last + 1] + st + toks[last + 1:]
    if k == 1:  # merge: T v;  ...decls...  v = e;  ->  ...decls... T v = e;
        c = []
        for d in ds:
            if not d["name"] or d["init"] or d["multi"] or not d["type"] or "static" in d["base"]: continue
            blk = [x for x in ds if x["block"] == d["block"]]
            last = max(x["end"] for x in blk)
            if toks[last + 1][1] == d["name"] and toks[last + 2][1] == "=":
                c.append((d, last))
        if not c: return None
        d, last = r.choice(c)
        e = last + 1
        while toks[e][1] != ";":
            if toks[e][1] in "([{": e = match_fwd(toks, e)
            e += 1
        if uses(toks, d["name"], last + 3, e): return None
        nd = [list(y) for y in toks[d["start"]:d["end"]]] + T(["="]) + [list(y) for y in toks[last + 3:e]] + T([";"])
        return toks[:d["start"]] + toks[d["end"] + 1:last + 1] + nd + toks[e + 1:]
    if k == 2:  # swap adjacent declarations
        c = [(x, y) for x, y in zip(ds, ds[1:]) if x["block"] == y["block"] and x["name"] and y["name"]
             and toks[x["end"] + 1:y["start"]] == []]
        c = [(x, y) for x, y in c if not (y["init"] and uses(toks, x["name"], *y["init"]))
             and not (x["init"] and y["init"] and side_effects(toks, *x["init"]) and side_effects(toks, *y["init"]))]
        if not c: return None
        x, y = r.choice(c)
        X = [list(z) for z in toks[x["start"]:x["end"] + 1]]; Y = [list(z) for z in toks[y["start"]:y["end"] + 1]]
        X[0][0], Y[0][0] = Y[0][0], X[0][0]
        return toks[:x["start"]] + Y + X + toks[y["end"] + 1:]
    # late copy of a parameter with a redundant cast (round 35)
    ps = [p for p, t in ctx.params.items() if t and p in {t_ for _, t_ in toks} and "_pc" not in p]
    if not ps: return None
    p = r.choice(ps); ty = ctx.params[p]
    body = [k_ for k_, x in enumerate(toks) if x[1] == "{"][0]
    top = [d for d in ds if d["block"] == body]
    pos = r.randrange(len(top) + 1)
    at = top[pos - 1]["end"] + 1 if pos else body + 1
    if any(uses(toks, p, d["start"], d["end"]) for d in top[pos:]): return None
    n = p + "_pc"
    nd = T([ty, n, "=", "(", ty, ")", p, ";"]); nd[0][0] = "\n    "
    rest = [[pre, n if t == p and toks[k_ - 1][1] not in (".", "->") else t] for k_, (pre, t) in enumerate(toks[at:], at)]
    return toks[:at] + nd + rest


def m_if(toks, ctx, r):
    c = [i for i, x in enumerate(toks) if x[1] == "if"]
    if not c: return None
    i = r.choice(c)
    h = match_fwd(toks, i + 1); b1 = body_of(toks, h + 1)
    if toks[b1 + 1][1] == "else":
        e1 = body_of(toks, b1 + 2)
        if toks[b1 + 2][1] == "if" and r.random() < 0.7: return None
        A = [list(y) for y in toks[h + 1:b1 + 1]]; B = [list(y) for y in toks[b1 + 2:e1 + 1]]
        cond = toks[i + 2:h]
        return toks[:i + 2] + T(["!", "("]) + [list(y) for y in cond] + T([")", ")"]) + B + [[toks[b1 + 1][0], "else"]] + A + toks[e1 + 1:]
    d, q = 0, None
    for k in range(i + 2, h):
        t = toks[k][1]
        if t in "([": d += 1
        elif t in ")]": d -= 1
        elif d == 0 and t == "||": return None
        elif d == 0 and t == "&&": q = k
    if q is None: return None
    body = [list(y) for y in toks[h + 1:b1 + 1]]
    return toks[:i + 2] + toks[i + 2:q] + T([")", "{", "if", "("]) + toks[q + 1:h + 1] + body + T(["}"]) + toks[b1 + 1:]


def m_incr(toks, ctx, r):
    c = []
    for i, x in enumerate(toks):
        if toks[i - 1][1] in (";", "{", "}", "(") and isid(x[1]):
            j = i + 1
            if toks[j][1] in ("++", "--") and toks[j + 1][1] in (";", ")"): c.append((i, j + 1, x[1], toks[j][1][0]))
            if toks[j][1] in ("+=", "-=") and toks[j + 1][1] == "1" and toks[j + 2][1] in (";", ")"): c.append((i, j + 2, x[1], toks[j][1][0]))
        if x[1] in ("++", "--") and isid(toks[i + 1][1]) and toks[i - 1][1] in (";", "{", "}", "(") and toks[i + 2][1] in (";", ")"):
            c.append((i, i + 2, toks[i + 1][1], x[1][0]))
    if not c: return None
    a, b, v, op = r.choice(c)
    if toks[b][1] == ")" and toks[a - 1][1] == "(": return None
    forms = [[v, op * 2], [op * 2, v], [v, op + "=", "1"], [v, "=", v, op, "1"]]
    f = T(r.choice(forms)); f[0][0] = toks[a][0]
    return toks[:a] + f + toks[b:]


MUTATIONS = [(m_commute, 3), (m_compound, 1), (m_shift, 1), (m_not, 1), (m_temp, 2), (m_untemp, 1), (m_cast, 3),
             (m_index, 2), (m_loop, 1), (m_decl, 3), (m_if, 1), (m_incr, 1)]


def mutate(toks, ctx, r, n):
    for _ in range(n * 4):
        f = r.choices([m for m, _ in MUTATIONS], [w for _, w in MUTATIONS])[0]
        try:
            t = f(toks, ctx, r)
        except (TypeError, IndexError, KeyError, ValueError):
            t = None
        if t is not None:
            toks = t; n -= 1
            if n == 0: break
    return toks


# ---------------------------------------------------------------- source handling and scoring
def typedef_names(module):
    s = set()
    paths = [os.path.join("include", f) for f in os.listdir("include")] + \
            [os.path.join("src", module, f) for f in os.listdir(os.path.join("src", module))]
    for p in paths:
        if not p.endswith((".h", ".c")): continue
        txt = open(p, errors="replace").read()
        s |= set(re.findall(r"\btypedef\b[^;{]*?(\w+)\s*(?:\[[^\]]*\])?\s*;", txt))
        s |= set(re.findall(r"\btypedef\b[^;]*?\(\s*\*\s*(\w+)\s*\)", txt))
        s |= set(re.findall(r"\}\s*(\w+)\s*;", txt))  # typedef struct {...} Name;
    return s - KW


def find_func(text, func):
    m = re.search(r"^[A-Za-z_][^;{}()\n]*\b%s\s*\(([^;{]*)\)\s*\{" % re.escape(func), text, re.M)
    if not m: raise SystemExit("%s: definition not found" % func)
    d, i = 0, m.end() - 1
    for j in range(i, len(text)):
        if text[j] == "{": d += 1
        elif text[j] == "}":
            d -= 1
            if d == 0: return m.start(), j + 1, m.group(1)
    raise SystemExit("unbalanced braces")


def parse_params(s):
    out = {}
    for p in s.split(","):
        m = re.match(r"\s*(.*?[\s*])(\w+)\s*$", p)
        if m and "[" not in p and "(" not in p:
            out[m.group(2)] = re.sub(r"\s+", " ", m.group(1).replace("*", " * ")).strip().replace(" *", "*").replace("* *", "**")
    return out


def orig_size(module, addr):
    nxt = None
    for m in re.finditer(r"^\w+ = (0x[0-9A-Fa-f]+);", open("symbols/%s.txt" % module).read(), re.M):
        v = int(m.group(1), 16)
        if v > addr and (nxt is None or v < nxt): nxt = v
    return nxt - addr


class Scorer:
    def __init__(self, module, path, func, workdir):
        self.module, self.func, self.dir = module, func, workdir
        sym = re.search(r"^%s = (0x[0-9A-Fa-f]+);" % func, open("symbols/%s.txt" % module).read(), re.M)
        self.addr = int(sym.group(1), 16)
        self.size = orig_size(module, self.addr)
        o = self.addr - BASE + SEG
        self.want = open("build/elf/%s.elf" % module, "rb").read()[o:o + self.size]
        self.text = open(path).read()
        self.a, self.b, self.params = find_func(self.text, func)
        self.flags = ["-O4,p", "-sdatathreshold", "0", "-i", "include", "-i", os.path.dirname(path), "-DNON_MATCHING"]

    def score(self, ftext, slot):
        src = os.path.join(self.dir, "w%02d.c" % slot); obj = src[:-2] + ".o"
        open(src, "w").write(self.text[:self.a] + ftext + self.text[self.b:])
        if os.path.exists(obj): os.remove(obj)
        r = subprocess.run(["tools/mwcc.sh", *self.flags, "-c", src, "-o", obj], capture_output=True, text=True)
        if r.returncode or not os.path.exists(obj): return None
        try:
            code, rel = object_function(obj, self.func)
        except SystemExit:
            return None
        d = 0
        for k in range(0, max(len(code), self.size), 4):
            if k + 4 > len(code) and k + 4 <= self.size:  # alignment padding after the function is not a difference
                d += struct.unpack_from("<I", self.want, k)[0] != 0; continue
            if k + 4 > len(code) or k + 4 > self.size: d += 1; continue
            w, v = struct.unpack_from("<I", code, k)[0], struct.unpack_from("<I", self.want, k)[0]
            t = rel.get(k)
            d += not ((w >> 26) == (v >> 26) if t == R_MIPS_26 else (w >> 16) == (v >> 16)
                      if t in (R_MIPS_HI16, R_MIPS_LO16, R_MIPS_GPREL16) else w == v)
        return d


def search(path, func, module, minutes, jobs, seed, start=None):
    wd = os.path.join("build", "tmp", "permute", func); os.makedirs(wd, exist_ok=True)
    sc = Scorer(module, path, func, wd)
    ftext = open(start).read() if start else sc.text[sc.a:sc.b]
    ctx = Ctx(typedef_names(module), parse_params(sc.params))
    base = sc.score(ftext, 0)
    log = open(os.path.join(wd, "log.txt"), "a")
    print("%s: base %s words (size %d)" % (func, base, sc.size), file=log, flush=True); print("base", base, flush=True)
    if base is None: raise SystemExit("base does not compile")
    pool = {ftext: base}; best = [base]; seen = set([ftext]); lock = threading.Lock(); n = [0]
    end = time.time() + minutes * 60

    def worker(slot):
        r = random.Random(seed * 1000 + slot)
        while time.time() < end and best[0] > 0:
            with lock:
                ranked = sorted(pool.items(), key=lambda kv: kv[1])
                par = ranked[0][0] if r.random() < 0.5 else r.choice(ranked[:16])[0]
            cand = render(mutate(lex(par), ctx, r, 1 + int(r.expovariate(1.2))))
            with lock:
                if cand in seen: continue
                seen.add(cand)
            s = sc.score(cand, slot)
            with lock:
                n[0] += 1
                if s is None: continue
                if len(pool) < 32 or s <= max(pool.values()):
                    pool[cand] = s
                    if len(pool) > 32: del pool[max(pool, key=pool.get)]
                if s < best[0]:
                    best[0] = s
                    open(os.path.join(wd, "score_%d.c" % s), "w").write(cand)
                    print("%s: %d words after %d candidates" % (func, s, n[0]), file=log, flush=True)
                    print(s, "after", n[0], flush=True)

    ts = [threading.Thread(target=worker, args=(k + 1,)) for k in range(jobs)]
    for t in ts: t.start()
    for t in ts: t.join()
    print("%s: done, best %d, %d candidates" % (func, best[0], n[0]), file=log, flush=True)
    print("best", best[0], "candidates", n[0])
    return best[0]


# ---------------------------------------------------------------- self-test
def selftest():
    ctx = Ctx({"Foo"}, {"p": "Foo*", "n": "int"})
    src = """{
    int i;
    int s = n * 4;
    unsigned int m = 0;
    Foo *q = p;

    for (i = 0; i < n; i++) {
        s += p->a[i] + q->b;
        if (!m && s == 0)
            m = (unsigned int)s << 1;
        else
            m = f(s, m) & 0xFF;
    }
    while (m != 0)
        m--;
    return s + m;
}"""
    t = lex(src)
    assert render(t) == src
    i = [k for k, x in enumerate(t) if x[1] == "+"][0]
    assert render(t[left_operand(t, i, 12, ctx):i]) .strip() == "p->a[i]"
    assert render(t[i + 1:right_operand(t, i, 12, ctx)]).strip() == "q->b"
    k = [k for k, x in enumerate(t) if x[1] == "<<"][0]
    assert render(t[left_operand(t, k, 11, ctx):k]).strip() == "(unsigned int)s"
    assert not is_binary(t, [k for k, x in enumerate(t) if x[1] == "*"][1], ctx)  # Foo *q
    seen = {}
    for name, f in [(f.__name__, f) for f, _ in MUTATIONS]:
        outs = set()
        for s in range(60):
            o = f(t, ctx, random.Random(s))
            if o is not None:
                txt = render(o); outs.add(txt)
                assert txt.count("{") == txt.count("}") and txt.count("(") == txt.count(")"), (name, txt)
        seen[name] = outs
        assert outs, name + " never applied"
    want = {"m_commute": "q->b + p->a[i]", "m_compound": "s = s +", "m_shift": "n << 2", "m_not": "(m == 0)",
            "m_temp": "__typeof__", "m_untemp": "((Foo*)( p))->b", "m_cast": "((int) s)", "m_index": "(*p).a",
            "m_loop": "while ( i < n)", "m_decl": "n_pc = ( int) n", "m_if": "else", "m_incr": "i += 1"}
    for name, w in want.items():
        assert any(w.replace(" ", "") in re.sub(r"\s", "", o) for o in seen[name]), (name, w, sorted(seen[name])[:3])
    # the scorer must report 0 for a function that already matches (needs the toolchain and build/elf)
    if os.path.exists("build/elf/BOOT.elf"):
        wd = os.path.join("build", "tmp", "permute", "_selftest"); os.makedirs(wd, exist_ok=True)
        sc = Scorer("BOOT", "src/BOOT/anim.c", "anim_eval", wd)
        assert sc.score(sc.text[sc.a:sc.b], 0) == 0
    print("permute.py: ok")


def main():
    if sys.argv[1:] == ["--selftest"]: return selftest()
    ap = argparse.ArgumentParser(usage=__doc__)
    ap.add_argument("src"); ap.add_argument("func")
    ap.add_argument("--minutes", type=float, default=30); ap.add_argument("-j", type=int, default=4)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--start", help="function text to start from (a score_N.c of an earlier run)")
    a = ap.parse_args()
    search(a.src, a.func, os.environ.get("MODULE", "BOOT"), a.minutes, a.j, a.seed, a.start)


if __name__ == "__main__":
    main()
