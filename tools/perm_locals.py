#!/usr/bin/env python3
"""perm_locals.py SRC.c FUNC [--apply]: try every order of FUNC's leading local declarations (the lines
up to the first blank line of its body), compile each with tools/check_file.py and print the best
`words-differ | declarations`. --apply writes the best order back into SRC.c. $MODULE as usual.

Declaration order decides mwccpsp's register allocation, so this often finishes a function whose
shape is right. Keep it to <= 7 declarations (n! compiles). For template-based variants of whole
expressions use tools/perm_decls.py. Sanity check: `python3 tools/perm_locals.py --selftest`."""
import itertools, os, re, subprocess, sys


def decl_block(s, fn):
    """(start, end) offsets of the declaration lines at the top of fn's body."""
    m = re.search(r'^[^\n]*\b' + re.escape(fn) + r'\([^\n]*\)\n\{\n', s, re.M)
    if not m:
        sys.exit(f'{fn}: definition not found')
    a = i = m.end()
    while s[i:s.index('\n', i)].strip():
        i = s.index('\n', i) + 1
    return a, i


def score(path, fn, module):
    out = subprocess.run(['python3', 'tools/check_file.py', module, path], capture_output=True, text=True).stdout
    m = re.search(r'^(OK |BAD) ' + re.escape(fn) + r'\s.*?(?:(\d+) words differ)?$', out, re.M)
    if not m:
        return 999
    return 0 if m.group(1) == 'OK ' else int(m.group(2) or 999)


def main(argv):
    if argv[:1] == ['--selftest']:
        s = 'int f(int a)\n{\n    int i;\n    int j;\n\n    return 0;\n}\n'
        a, b = decl_block(s, 'f')
        assert s[a:b] == '    int i;\n    int j;\n', s[a:b]
        print('ok')
        return
    src, fn, module = argv[0], argv[1], os.environ.get('MODULE', 'BOOT')
    s = open(src).read()
    a, b = decl_block(s, fn)
    lines = s[a:b].splitlines(keepends=True)
    os.makedirs('build/tmp', exist_ok=True)
    tmp = 'build/tmp/perm_locals.c'
    best = (999, lines)
    for perm in itertools.permutations(lines):
        open(tmp, 'w').write(s[:a] + ''.join(perm) + s[b:])
        n = score(tmp, fn, module)
        if n < best[0]:
            best = (n, list(perm))
        if n == 0:
            break
    print(best[0], ''.join(best[1]).replace('\n', ' | '))
    if '--apply' in argv and best[0] < 999:
        open(src, 'w').write(s[:a] + ''.join(best[1]) + s[b:])


if __name__ == '__main__':
    main(sys.argv[1:])
