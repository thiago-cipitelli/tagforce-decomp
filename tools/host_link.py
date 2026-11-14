#!/usr/bin/env python3
"""Link every module into one 32-bit host executable, build/host/tagforce (port step 4a, docs/port_survey.md).

  1. compile each src/<m>/*.c with the host gcc (NON_MATCHING drafts)   -> build/host/obj/<m>/*.o
  2. per module: weaken the symbols that several of its objects define (gnu89 `inline` in headers emits a copy
     in each object) and globalize the statics that other files of the module (or its data) reference
  3. per module: data from the ELF (tools/host_data.py)                  -> build/host/data/<m>.s/.o
  4. per module: ld -r of everything (+ --defsym for the aliases inside C-defined tables) -> build/host/mod/<m>.o
  5. names defined by more than one module: kept in BOOT, renamed <m>_name in each gmodule (references from other
     gmodules follow their `import:<lib>` line); the table goes to platform/pc/modules.c
  6. platform/pc/stubs.c (tools/host_stubs.py) for the firmware imports, link with -lm
Prints the count of undefined and duplicate symbols (0/0 is the gate).

usage: host_link.py [-j N]
"""
import concurrent.futures as cf
import glob
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
import host_data  # noqa: E402

# -mno-sse: the floats are x87 anyway (-mfpmath=387); without SSE gcc never copies a 16-aligned type (Mat4) with
# movaps, which faults where a caller declares the same object without the alignment
CFLAGS = ["-m32", "-mno-sse", "-std=gnu89", "-w", "-O0", "-g", "-fno-common", "-fno-pie", "-DNON_MATCHING", "-Iinclude"]
# platform/pc sees include/ only through "": include/stdarg.h would shadow the system one under <stdarg.h>
PCFLAGS = [f for f in CFLAGS if f != "-Iinclude"] + ["-iquote", "include", "-pthread", "-O2", "-I/usr/include/freetype2"]
# window and GL (platform/pc/window.c, ge.c): the system's 32-bit libX11 and libGL; sound: 32-bit libasound (audio.c). The version script exports
# nothing: the game defines ~50 libc/libm names (malloc, exit, mtx_init - a matrix function -, strtod, sinf...) that
# would otherwise interpose glibc's for every shared library too (Mesa crashed in the game's mtx_init)
LIBS = ["-lX11", "-lGL", "-lfreetype", "-lasound", "-lm", "-Wl,--version-script=build/host/hide.ver"]
OUT = "build/host"


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode:
        raise SystemExit(f"{' '.join(cmd)}\n{r.stdout}{r.stderr}")
    return r.stdout


def compile_one(src, obj):
    deps = [src] + glob.glob("include/*.h") + glob.glob(os.path.dirname(src) + "/*.h")
    if os.path.exists(obj) and os.path.getmtime(obj) >= max(os.path.getmtime(d) for d in deps):
        return
    os.makedirs(os.path.dirname(obj), exist_ok=True)
    run(["gcc"] + CFLAGS + ["-I" + os.path.dirname(src), "-c", src, "-o", obj])


def nm(path):
    """[(name, type letter, size)]; size 0 when nm does not print one."""
    out = []
    for line in run(["nm", "-S", path]).splitlines():
        p = line.split()
        if len(p) == 4:
            out.append((p[3], p[2], int(p[1], 16)))
        elif len(p) == 3:
            out.append((p[2], p[1], 0))
        else:
            out.append((p[1], p[0], 0))
    return out


def prepare_module(m, objs, pdefs):
    """Steps 2-4 for one module; returns the module object path and a log of what was changed."""
    log = []
    syms = {o: nm(o) for o in objs}
    gdef, ldef, undef = {}, {}, set()
    for o, l in syms.items():
        for n, t, sz in l:
            if t == "U":
                undef.add(n)
            elif t in "TDBRVW":
                gdef.setdefault(n, []).append((o, sz))
            elif t in "tdbr":
                ldef.setdefault(n, []).append((o, t, sz))
    # header `inline` copies: one definition per object; keep them all weak
    for n, l in gdef.items():
        if len(l) > 1:
            for o, _sz in l:
                run(["objcopy", "--weaken-symbol=" + n, o])
            log.append(f"weak {n} ({len(l)} objects)")
    funcs = set(gdef) | pdefs | {n for n, l in ldef.items() if len(l) == 1 and l[0][1] == "t"}
    cdefs = {n: l[0][1] for n, l in gdef.items()}
    # file statics with a unique name: the data may point at them too (globalized below when it does)
    cdefs.update({n: l[0][2] for n, l in ldef.items() if len(l) == 1 and l[0][1] in "dbr" and n not in cdefs})
    asm, defsyms, st = host_data.generate(m, cdefs, funcs, undef - set(gdef))
    os.makedirs(f"{OUT}/data", exist_ok=True)
    open(f"{OUT}/data/{m}.s", "w").write(asm)
    run(["as", "--32", f"{OUT}/data/{m}.s", "-o", f"{OUT}/data/{m}.o"])
    dundef = {n for n, t, _sz in nm(f"{OUT}/data/{m}.o") if t == "U"}
    dlabels = {n for n, t, _sz in nm(f"{OUT}/data/{m}.o") if t in "DB"}
    # statics referenced from another file of the module or from the data: globalize when unique
    for n in sorted((undef | dundef) - set(gdef) - dlabels - set(defsyms)):
        l = ldef.get(n, [])
        if len(l) == 1:
            run(["objcopy", "--globalize-symbol=" + n, l[0][0]])
            log.append(f"global {n} ({os.path.basename(l[0][0])})")
    os.makedirs(f"{OUT}/mod", exist_ok=True)
    out = f"{OUT}/mod/{m}.o"
    run(["ld", "-m", "elf_i386", "-r", "-o", out] + [f"--defsym={k}={v}" for k, v in sorted(defsyms.items())]
        + objs + [f"{OUT}/data/{m}.o"])
    log.append(f"data {st}")
    return out, log


def import_libs(m):
    """name -> lib of the import lines of symbols/<m>.txt."""
    r = {}
    for line in open(f"symbols/{m}.txt"):
        g = re.match(r"^(\w+) = 0x\w+; // type:\w+ import:(\w+)", line)
        if g:
            r[g.group(1)] = g.group(2)
    return r


def main():
    jobs = int(sys.argv[sys.argv.index("-j") + 1]) if "-j" in sys.argv else os.cpu_count()
    mods = sorted(os.path.basename(p)[:-4] for p in glob.glob("build/elf/*.elf"))
    srcs = {m: sorted(glob.glob(f"src/{m}/*.c")) for m in mods}
    objs = {m: [f"{OUT}/obj/{m}/{os.path.basename(s)[:-2]}.o" for s in srcs[m]] for m in mods}
    stamp = f"{OUT}/obj/cflags"   # the compile cache goes by date: start over when the flags change
    if not os.path.exists(stamp) or open(stamp).read() != " ".join(CFLAGS):
        subprocess.run(["rm", "-rf", f"{OUT}/obj"])
        os.makedirs(f"{OUT}/obj")
        open(stamp, "w").write(" ".join(CFLAGS))
    run([sys.executable, "tools/host_stubs.py", "platform/pc/stubs.c"])
    pobjs = []
    for p in sorted(glob.glob("platform/pc/*.c")):
        o = f"{OUT}/obj/platform/{os.path.basename(p)[:-2]}.o"
        os.makedirs(os.path.dirname(o), exist_ok=True)
        run(["gcc"] + PCFLAGS + ["-c", p, "-o", o])
        pobjs.append(o)
    pdefs = {n for o in pobjs for n, t, _sz in nm(o) if t in "TDBR"}
    # the game's newlib (src/BOOT) defines these on its own FILE layout and interposes glibc's: a platform file
    # that calls one gets the game's (a load that read 0 bytes); use read()/write() or fprintf there
    game_stdio = {"fread", "fwrite", "fflush", "fgets", "fputs", "fseek", "ftell", "getc", "putc", "ungetc", "vfprintf"}
    bad = sorted((o, n) for o in pobjs for n, t, _sz in nm(o) if t == "U" and n in game_stdio
                 and re.search(rf"^{n} = ", open("symbols/BOOT.txt").read(), re.M))
    if bad:
        raise SystemExit("".join(f"{o}: calls {n}, which binds to the game's newlib\n" for o, n in bad))
    with cf.ThreadPoolExecutor(jobs) as ex:
        list(ex.map(lambda a: compile_one(*a), [(s, o) for m in mods for s, o in zip(srcs[m], objs[m])]))
        # objcopy edits the objects in place: start from fresh copies every run
        res = dict(zip(mods, ex.map(lambda m: prepare_module(m, fresh(objs[m]), pdefs), mods)))
    with open(f"{OUT}/host_link.log", "w") as f:
        for m in mods:
            f.write("".join(f"{m}: {x}\n" for x in res[m][1]))

    # step 5: cross-module duplicates
    defs, refs = {}, {}
    for m in mods:
        for n, t, _sz in nm(res[m][0]):
            if t in "TDBRV":
                defs.setdefault(n, []).append(m)
            elif t == "U":
                refs.setdefault(n, []).append(m)
    renames = {m: {} for m in mods}
    for n, ms in defs.items():
        if len(ms) > 1:
            for m in ms:
                if m != "BOOT":
                    renames[m][n] = f"{m}_{n}"
    for m in mods:
        imp = import_libs(m)
        for n in [n for n in defs if any(n in renames[x] for x in mods)]:
            if m in refs.get(n, []) and imp.get(n) in renames and n in renames[imp[n]]:
                renames[m][n] = renames[imp[n]][n]
    for m in mods:
        if renames[m]:
            run(["objcopy"] + [f"--redefine-sym={a}={b}" for a, b in sorted(renames[m].items())] + [res[m][0]])
    if gen_modules_c(mods, renames):
        run(["gcc"] + PCFLAGS + ["-c", "platform/pc/modules.c", "-o", f"{OUT}/obj/platform/modules.o"])
        if f"{OUT}/obj/platform/modules.o" not in pobjs:
            pobjs.append(f"{OUT}/obj/platform/modules.o")
    allo = [res[m][0] for m in mods] + pobjs
    open(f"{OUT}/hide.ver", "w").write("{ local: *; };\n")
    r = subprocess.run(["gcc", "-m32", "-no-pie", "-pthread", "-o", f"{OUT}/tagforce"] + allo + LIBS, capture_output=True, text=True)
    open(f"{OUT}/link.log", "w").write(r.stderr)
    und = sorted(set(re.findall(r"undefined reference to `([^']+)'", r.stderr)))
    dup = sorted(set(re.findall(r"multiple definition of `([^']+)'", r.stderr)))
    open(f"{OUT}/undef.txt", "w").write("".join(n + "\n" for n in und))
    print(f"host-link: {len(und)} undefined, {len(dup)} duplicate"
          + (f" -> {OUT}/tagforce" if r.returncode == 0 else f" (see {OUT}/link.log)"))
    return 0 if r.returncode == 0 else 1


def fresh(objs):
    """Copy the compiled objects (compile cache) to the objects the edits work on."""
    out = []
    for o in objs:
        w = o.replace(f"{OUT}/obj/", f"{OUT}/work/")
        os.makedirs(os.path.dirname(w), exist_ok=True)
        with open(o, "rb") as a, open(w, "wb") as b:
            b.write(a.read())
        out.append(w)
    return out


def gen_modules_c(mods, renames):
    """platform/pc/modules.c: per gmodule, the host names of the symbols that had to be renamed (entry points)."""
    lines = ["/* generated by tools/host_link.py: names of the gmodule symbols that clash between modules and were",
             "   renamed <module>_<name> in the host link (the PSP resolves them per module). */",
             "typedef struct { const char *module, *name; void *addr; } HostModSym;", ""]
    ent = [(m, a, b) for m in mods for a, b in sorted(renames[m].items()) if b == f"{m}_{a}"]
    lines += [f"extern char {b}[];" for _m, _a, b in ent]
    lines += ["", "const HostModSym host_mod_syms[] = {"]
    lines += [f'    {{"{m}", "{a}", {b}}},' for m, a, b in ent]
    lines += ["    {0, 0, 0}", "};", ""]
    text = "\n".join(lines)
    if not os.path.exists("platform/pc/modules.c") or open("platform/pc/modules.c").read() != text:
        open("platform/pc/modules.c", "w").write(text)
        return True
    return False


if __name__ == "__main__":
    sys.exit(main())
