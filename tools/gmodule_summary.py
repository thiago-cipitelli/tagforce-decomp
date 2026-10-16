#!/usr/bin/env python3
"""Summarize each exported gmodule decompilation (build/decomp/<m>/index.tsv + module_entry.c) as a Markdown table:
role (scene/library), scene main/exit, function count, biggest function, distinct libehsys calls, top engine calls.
usage: gmodule_summary.py build/decomp symbols > docs/gmodules.md"""
import collections
import glob
import os
import re
import sys

dec, symdir = sys.argv[1], sys.argv[2]
boot = {int(m.group(2), 16): m.group(1) for m in re.finditer(r"(\w+)\s*=\s*0x([0-9A-Fa-f]+);", open(os.path.join(symdir, "BOOT.txt")).read())}


def ehsys_name(tok):
    m = re.match(r"ehsys_([0-9A-F]{8})$", tok)
    return boot.get(int(m.group(1), 16), tok) if m else tok


print("| Módulo | Papel | main / exit da cena | Funções | Maior função | libehsys distintas | Chamadas de engine mais frequentes |")
print("|---|---|---|---|---|---|---|")
for d in sorted(glob.glob(os.path.join(dec, "lib*"))):
    m = os.path.basename(d)
    idx = [l.split("\t") for l in open(os.path.join(d, "index.tsv")).read().splitlines()[1:]]
    entry = ""
    for cand in ("module_start.c", "entry.c", "module_entry.c"):
        if os.path.exists(os.path.join(d, cand)):
            entry = open(os.path.join(d, cand)).read()
            break
    reg = re.search(r"ehscript_register\((\w+),\s*&?(\w+)", entry) or re.search(r"ehsys_0883D0F8\((\w+),\s*&?(\w+)", entry)
    role = "cena" if reg else ("biblioteca" if "return 0" in entry else "?")
    mainexit = f"`{reg.group(1)}` / `{reg.group(2)}`" if reg else "—"
    biggest = max(idx, key=lambda r: int(r[2]))
    calls = collections.Counter()
    for r in idx:
        for c in r[3].split(","):
            if c.startswith("ehsys_") or c in boot.values():
                calls[ehsys_name(c)] += 1
    distinct = sum(1 for k in calls if not k.startswith("FUN_"))
    top = ", ".join(f"{k} ({v})" for k, v in calls.most_common(6) if k not in ("memset", "memcpy"))
    print(f"| {m} | {role} | {mainexit} | {len(idx)} | `{biggest[1]}` ({biggest[2]} B) | {distinct} | {top} |")
