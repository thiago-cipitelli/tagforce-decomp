#!/usr/bin/env python3
"""Work queue for matching the functions kept in @asm (docs/matching.md). Reads the objdiff report
(build/objdiff/report.json: tools/objdiff_gen.py, then tools/bytematch.py) and writes docs/matching_queue.tsv:
every function with C that does not match yet, easiest first (highest fuzzy %, then smallest).
Left out: GCC-compiled SDK files of BOOT (files with `__MWERKS__` asm functions, a separate track) and the
functions in docs/matching_skip.tsv (known unfixable, so no session retries them).
usage: asm_queue.py                    regenerate docs/matching_queue.tsv from the report
       asm_queue.py --next [N] [MOD]   print the next N entries (default 10), optionally of one module
       asm_queue.py --check            sanity check of the committed queue"""
import json, os, re, sys

QUEUE, SKIP, REPORT = "docs/matching_queue.tsv", "docs/matching_skip.tsv", "build/objdiff/report.json"
HEAD = "fuzzy\tsize\tmodule\tfunction\taddress\tfile"


def skipped():
    if not os.path.exists(SKIP):
        return set()
    return {l.split("\t")[0] for l in open(SKIP) if l.strip() and not l.startswith("#")}


def addresses(module):
    t = open(f"symbols/{module}.txt").read()
    return {n: a for n, a in re.findall(r"^(\w+) = (0x[0-9A-Fa-f]+);", t, re.M)}


def build():
    rep, skip, sdk, addr, rows = json.load(open(REPORT)), skipped(), {}, {}, []
    for u in rep["units"]:
        src = (u.get("metadata") or {}).get("source_path")
        if not src or not os.path.exists(src):
            continue
        if src not in sdk:
            t = open(src, errors="replace").read()  # GCC SDK: mwcc asm functions, or one @asm over the whole @range
            r = re.search(r"^// @range (\S+)", t, re.M)
            sdk[src] = "__MWERKS__" in t or bool(r and re.search(r"^// @asm " + re.escape(r.group(1)) + "$", t, re.M))
        if sdk[src]:
            continue
        mod = src.split("/")[1]
        if mod not in addr:
            addr[mod] = addresses(mod)
        for f in u.get("functions", []):
            fz = f.get("fuzzy_match_percent", 0)
            if fz < 100 and f["name"] not in skip:
                m = re.fullmatch(r"func_([0-9A-F]{8})", f["name"])  # unnamed: the address is in the name
                a = addr[mod].get(f["name"]) or (m and "0x" + m.group(1)) or "?"
                rows.append((fz, int(f["size"]), mod, f["name"], a, src))
    rows.sort(key=lambda r: (-r[0], r[1]))
    with open(QUEUE, "w") as o:
        o.write(HEAD + "\n")
        for r in rows:
            o.write(f"{r[0]:.1f}\t{r[1]}\t{r[2]}\t{r[3]}\t{r[4]}\t{r[5]}\n")
    print(f"{len(rows)} functions -> {QUEUE}")


def entries():
    return [l.rstrip("\n").split("\t") for l in open(QUEUE).readlines()[1:]]


def main(a):
    if not a:
        return build()
    if a[0] == "--next":
        n = int(a[1]) if len(a) > 1 else 10
        mod = a[2] if len(a) > 2 else None
        skip = skipped()
        for e in [e for e in entries() if (not mod or e[2] == mod) and e[3] not in skip][:n]:
            print("\t".join(e))
        return
    if a[0] == "--check":
        es = entries()
        assert open(QUEUE).readline().rstrip("\n") == HEAD, "bad header"
        assert all(len(e) == 6 and os.path.exists(e[5]) for e in es), "row without a source file"
        assert all(e[4] != "?" for e in es), "row without an address in symbols"
        fz = [float(e[0]) for e in es]
        assert fz == sorted(fz, reverse=True), "queue not sorted by fuzzy"
        assert not skipped() & {e[3] for e in es}, "skipped function still in the queue"
        print(f"asm_queue.py: ok ({len(es)} entries)")
        return
    sys.exit(__doc__)


main(sys.argv[1:])
