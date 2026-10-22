#!/usr/bin/env python3
"""Build objdiff target/base objects and objdiff.json for progress reports (objdiff-cli report generate).
Target objects: the module re-split by splat with every `c`/`.data` segment turned into asm/data, so each
`src/<m>/x.c` gets a relocatable object with the original code (+ its `@asm` functions and `@data`) under the
same symbol names. Base objects: the `.c` compiled with -DNON_MATCHING, so functions kept in `@asm` show their
fuzzy score (falls back to the plain build when a draft does not compile). Asm-only segments become units with
no base (count as unmatched). usage: objdiff_gen.py [MODULE...]   (default: every build/elf/*.elf)"""
import glob, json, os, re, struct, subprocess, sys

PSPDEV = os.environ.get("PSPDEV", os.path.expanduser("~/opt/pspdev"))
AS = [f"{PSPDEV}/bin/psp-as", "-march=allegrex", "-mabi=eabi", "-EL", "-I", "include"]
LD = f"{PSPDEV}/bin/psp-ld"
MWCC = ["tools/mwcc.sh", "-O4,p", "-sdatathreshold", "0", "-i", "include"]
SEG = re.compile(r"^(\s*- \[0x[0-9A-Fa-f]+, )(c|asm|\.data|data), (\w+)\]", re.M)


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode:
        sys.exit(f"failed: {' '.join(cmd)}\n{r.stdout}{r.stderr}")
    return r


def unit_of(name, c_units):
    """C unit owning an asm segment name (`act_cond2_asm_0886F23C` -> `act_cond2`), longest name first."""
    for c in sorted(c_units, key=len, reverse=True):
        if re.fullmatch(rf"{re.escape(c)}((__rodata)?__\d+|_asm_[0-9A-Fa-f]{{8}})", name):
            return c
    return None


def base_refs(obj, addr):
    """References of `obj` to undefined symbols with a known address: ({address: {"sym+0xoff"}}, {names used}).
    REL addends are read from the instructions (HI16 paired with the next LO16 of the same symbol)."""
    b = open(obj, "rb").read()
    shoff, shentsize, shnum = struct.unpack_from("<I10xHH", b, 0x20)
    sh = [struct.unpack_from("<10I", b, shoff + i * shentsize) for i in range(shnum)]
    refs, names = {}, set()
    for _, typ, _, _, off, size, link, info, _, _ in sh:
        if typ != 9:  # SHT_REL
            continue
        symtab, strtab, tgt = sh[link], sh[sh[link][6]], sh[info][4]
        pending = {}
        for r_off, r_info in struct.iter_unpack("<II", b[off:off + size]):
            name_off, _, _, _, shndx = struct.unpack_from("<IIIBxH", b, symtab[4] + (r_info >> 8) * 16)
            name = b[strtab[4] + name_off:b.index(b"\0", strtab[4] + name_off)].decode()
            if shndx or name not in addr:  # defined in the object, or no known address
                continue
            names.add(name)
            word, kind = struct.unpack_from("<I", b, tgt + r_off)[0], r_info & 0xFF
            if kind == 5:  # R_MIPS_HI16
                pending.setdefault(name, []).append(word & 0xFFFF)
                continue
            if kind == 6:  # R_MIPS_LO16
                lo = (word & 0xFFFF) - ((word & 0x8000) << 1)
                addends = [(h << 16) + lo for h in pending.pop(name, [])] or [lo]
            elif kind == 2:  # R_MIPS_32
                addends = [word]
            else:
                continue
            for a in addends:
                if a and addr[name]:
                    refs.setdefault((addr[name] + a) & 0xFFFFFFFF, set()).add(f"{name}+0x{a:X}")
    return refs, names


def align_refs(files, obj, addr):
    """Rewrite target asm references (`D_08B0B688`, a named field inside a struct) to the `sym+off` spelling
    the base object uses for the same address, so objdiff sees equal relocations. Names the base uses stay."""
    refs, names = base_refs(obj, addr)

    def sub(g):
        n = g.group(0)
        a = int(n[2:], 16) if re.fullmatch(r"D_[0-9A-F]{8}", n) else addr.get(n)
        r = refs.get(a, ())
        return next(iter(r)) if len(r) == 1 and n not in names else n
    for f in files:
        s = open(f).read()
        t = re.sub(r"(?<=%hi\()\w+|(?<=%lo\()\w+|(?<=\.word )\w+", sub, s)
        if t != s:
            open(f, "w").write(t)


def module(m):
    out = f"build/objdiff/{m}"
    yaml = open(f"config/{m}.yaml").read()
    c_units = sorted(set(re.findall(r"^\s*- \[0x[0-9A-Fa-f]+, c, (\w+)\]", yaml, re.M)))
    n = iter(range(10 ** 6))
    # every C piece becomes a uniquely named asm/data segment (splat would overwrite repeated names)
    yaml = SEG.sub(lambda g: g.group(1) + {"c": "asm", ".data": "data"}.get(g.group(2), g.group(2)) + ", "
                   + (f"{g.group(3)}__{next(n)}" if g.group(2) in ("c", ".data") else g.group(3)) + "]", yaml)
    yaml = yaml.replace("base_path: ..", "base_path: ../..")
    for key, path in (("asm_path", "asm"), ("asset_path", "assets"), ("src_path", "src"), ("ld_script_path", "x.ld"),
                      ("undefined_funcs_auto_path", "uf.txt"), ("undefined_syms_auto_path", "us.txt")):
        yaml = re.sub(rf"({key}: )\S+", rf"\g<1>{out}/{path}", yaml)
    run(["rm", "-rf", out])
    os.makedirs(f"{out}/target", exist_ok=True)
    open(f"build/objdiff/{m}.yaml", "w").write(yaml)
    open(f"build/{m}.syms.txt", "w").write(run(["python3", "tools/splat_symbols.py", m]).stdout)
    run([".venv/bin/splat", "split", f"build/objdiff/{m}.yaml"])
    files = glob.glob(f"{out}/asm/**/*.s", recursive=True)
    for f in files:
        s = open(f).read()
        open(f, "w").write(re.sub(r"^\.set gp=64\n", "", s, flags=re.M))
    run(["python3", "tools/fix_asm.py", *AS, "-I", f"{out}/asm", "--", *files])
    addr = {n: int(a, 16) for n, a in re.findall(r"^(\w+) = (0x[0-9A-Fa-f]+);", open(f"build/{m}.syms.txt").read(), re.M)}

    groups = {}
    for f in files:
        name = os.path.basename(f).split(".")[0]
        if name.startswith("sceStub") or name == "bss":
            continue
        groups.setdefault(unit_of(name, c_units) or name, []).append(f)
    units = []
    for name, srcs in sorted(groups.items()):
        unit = {"name": f"{m}/{name}", "metadata": {"progress_categories": [m]}}
        if name in c_units:
            src, base = f"src/{m}/{name}.c", f"{out}/base/{name}.o"
            os.makedirs(os.path.dirname(base), exist_ok=True)
            r = subprocess.run([*MWCC, "-DNON_MATCHING", "-c", src, "-o", base], capture_output=True)
            if r.returncode:
                print(f"{src}: NON_MATCHING draft does not compile, using the matching build")
                run([*MWCC, "-c", src, "-o", base])
            unit["base_path"] = base
            unit["metadata"]["source_path"] = src
            align_refs(srcs, base, addr)
            for s in srcs:  # `@rodata` pieces sit in the ELF's .data; mwcc emits them in .rodata
                if "__rodata__" in s:
                    open(s, "w").write(open(s).read().replace(".section .data", ".section .rodata"))
        objs = []
        for s in sorted(srcs):
            o = f"{out}/target/{os.path.basename(s)}.o"
            run([*AS, "-I", f"{out}/asm", "-o", o, s])
            objs.append(o)
        unit["target_path"] = f"{out}/target/{name}.o"
        run([LD, "-r", "-o", unit["target_path"], *objs])
        units.append(unit)
    return units


def main():
    mods = sys.argv[1:] or sorted(os.path.basename(f)[:-4] for f in glob.glob("build/elf/*.elf"))
    units = []
    for m in mods:
        print(f"{m}...", flush=True)
        units += module(m)
    cfg = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "build_target": False,
        "build_base": False,
        "units": units,
        "progress_categories": [{"id": m, "name": m} for m in mods],
    }
    json.dump(cfg, open("objdiff.json", "w"), indent=1)
    print(f"objdiff.json: {len(units)} units")


if __name__ == "__main__":
    main()
