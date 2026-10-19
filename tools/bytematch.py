#!/usr/bin/env python3
"""objdiff progress report for ByteMatch (bytematch.dev). Runs objdiff-cli on objdiff.json (tools/objdiff_gen.py),
writes functional.json (functions with C under NON_MATCHING that do not match: fuzzy score between 0 and 100 in a
unit with a source file) and packs build/objdiff/bytematch.zip like a CI artifact. With BYTEMATCH_TOKEN and
BYTEMATCH_PROJECT=owner/repo set, uploads it for the current commit (version ULUS-10136).
usage: bytematch.py   (prints the overall and per-module progress)"""
import json, os, subprocess, sys, urllib.parse, urllib.request, zipfile

OUT = "build/objdiff"
VERSION = "ULUS-10136"

subprocess.run(["tools/ext/objdiff-cli", "report", "generate", "-o", f"{OUT}/report.json"], check=True, capture_output=True)
report = json.load(open(f"{OUT}/report.json"))
functional = [{"name": f["name"]} for u in report["units"] if u.get("metadata", {}).get("source_path")
              for f in u.get("functions", []) if 0 < f.get("fuzzy_match_percent", 0) < 100]
json.dump({"functions": functional}, open(f"{OUT}/functional.json", "w"))
with zipfile.ZipFile(f"{OUT}/bytematch.zip", "w", zipfile.ZIP_DEFLATED) as z:
    z.write(f"{OUT}/report.json", "report.json")
    z.write(f"{OUT}/functional.json", "functional.json")

# sanity: the pipeline-proof module must come out fully matched (make MODULE=libhtml_view check -> MATCH)
for u in report["units"]:
    if u["name"] in ("libhtml_view/lang_remap", "libhtml_view/save_path"):
        assert u["measures"].get("matched_code_percent") == 100, f"{u['name']}: objdiff disagrees with check_file"

for name, m in [("total", report["measures"])] + [(c["id"], c["measures"]) for c in report["categories"]]:
    print(f"{name:18s} {m.get('matched_code_percent', 0):5.1f}% code  {m.get('fuzzy_match_percent', 0):5.1f}% fuzzy  "
          f"{m.get('matched_functions', 0)}/{m.get('total_functions', 0)} functions")
print(f"{len(functional)} functions with non-matching C -> {OUT}/bytematch.zip")

token, project = os.environ.get("BYTEMATCH_TOKEN"), os.environ.get("BYTEMATCH_PROJECT")
if token and project:
    git = lambda *a: subprocess.run(["git", *a], capture_output=True, text=True, check=True).stdout.strip()
    if git("status", "--porcelain", "--untracked-files=no", "src", "include"):
        sys.exit("src/ or include/ has uncommitted changes: the report would not match the commit")
    q = urllib.parse.urlencode({"version": VERSION, "commit": git("rev-parse", "HEAD"),
                                "commit_message": git("log", "-1", "--format=%s"), "commit_timestamp": git("log", "-1", "--format=%cI")})
    req = urllib.request.Request(f"https://bytematch.dev/api/v1/projects/{project}/reports?{q}",
                                 data=open(f"{OUT}/bytematch.zip", "rb").read(), method="POST",
                                 headers={"Authorization": f"Bearer {token}", "Content-Type": "application/zip"})
    with urllib.request.urlopen(req) as r:
        print("uploaded:", r.status, r.read().decode()[:200])
