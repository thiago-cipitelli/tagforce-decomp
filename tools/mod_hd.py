#!/usr/bin/env python3
"""Install the "Tag Force 1 HD" texture pack (made for PPSSPP) as a port mod in mods/hd.

The pack's ULUS10136 folder holds the PNGs (backgrounds, cards, intro, npcs, shop, ui) and one textures.ini per card
style in "_PICK YOUR CARD STYLE HERE" (official = the TCG card art, anime = the art seen in the anime; the root
textures.ini is the official one). This writes mods/hd/textures.ini of the chosen style and links the image folders
(symlinks: the pack is 1.5 GB; --copy copies them instead, so the pack can be deleted afterwards).

It also writes mods/mods.txt when missing (or adds hd to it), with hd LAST: smaller mods such as mods/houses
(tools/mod_houses.py) then win over the pack's own versions of the same textures.

usage: mod_hd.py PACK_DIR {official,anime} [--copy] [MODS_DIR]   (PACK_DIR: the ULUS10136 folder or the one above it)
       mod_hd.py --check"""
import os, shutil, sys, tempfile

STYLES = {"official": "official_tcg_style", "anime": "anime_style"}
PICK = "_PICK YOUR CARD STYLE HERE"


def install(pack, style, copy=False, mods="mods"):
    if os.path.isdir(os.path.join(pack, "ULUS10136")):
        pack = os.path.join(pack, "ULUS10136")
    pack = os.path.abspath(pack)
    ini = os.path.join(pack, PICK, STYLES[style], "textures.ini")
    if not os.path.exists(ini):
        raise SystemExit(f"{ini} not found: PACK_DIR must be the pack's ULUS10136 folder")
    out = os.path.join(mods, "hd")
    if os.path.islink(out) or os.path.isfile(out):
        os.remove(out)
    elif os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(out)
    shutil.copy(ini, os.path.join(out, "textures.ini"))
    for d in sorted(os.listdir(pack)):
        src = os.path.join(pack, d)
        if os.path.isdir(src) and d != PICK:
            (shutil.copytree if copy else os.symlink)(src, os.path.join(out, d))
    order = os.path.join(mods, "mods.txt")
    if os.path.exists(order):
        names = [l.strip() for l in open(order) if l.strip() and l.strip() != "hd"]
    else:
        names = sorted(d for d in os.listdir(mods) if d != "hd" and not d.startswith(".")
                       and os.path.isdir(os.path.join(mods, d)))
    open(order, "w").write("\n".join(names + ["hd"]) + "\n")
    n = sum(1 for l in open(ini) if len(l) > 24 and l[24:27] == " = ")
    print(f"{out}: {n} textures, {style} card style ({'copied' if copy else 'linked to ' + pack}); {order}: "
          + ", ".join(names + ["hd"]))
    return n


def check():
    with tempfile.TemporaryDirectory() as t:
        pack = os.path.join(t, "TF1", "ULUS10136")
        for style in STYLES.values():
            os.makedirs(os.path.join(pack, PICK, style))
            open(os.path.join(pack, PICK, style, "textures.ini"), "w").write(
                "[options]\nhash = xxh64\n[hashes]\n0000000077b85817201e4e0c = cards/" + style + ".png\n")
        os.makedirs(os.path.join(pack, "cards"))
        mods = os.path.join(t, "mods")
        os.makedirs(os.path.join(mods, "houses"))
        assert install(os.path.join(t, "TF1"), "anime", False, mods) == 1
        assert "anime_style" in open(os.path.join(mods, "hd", "textures.ini")).read()
        assert os.path.islink(os.path.join(mods, "hd", "cards")) and not os.path.exists(os.path.join(mods, "hd", PICK))
        assert open(os.path.join(mods, "mods.txt")).read() == "houses\nhd\n"
        install(pack, "official", True, mods)   # again, copying: hd stays last and once
        assert not os.path.islink(os.path.join(mods, "hd", "cards"))
        assert open(os.path.join(mods, "mods.txt")).read() == "houses\nhd\n"
    print("mod_hd.py: ok")


if __name__ == "__main__":
    a = [x for x in sys.argv[1:] if x != "--copy"]
    if a == ["--check"]:
        check()
    elif len(a) in (2, 3) and a[1] in STYLES:
        install(a[0], a[1], "--copy" in sys.argv, *a[2:])
    else:
        raise SystemExit(__doc__)
