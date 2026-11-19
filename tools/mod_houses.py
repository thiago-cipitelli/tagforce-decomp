#!/usr/bin/env python3
"""Install the protagonist outfit mod ("GXTagForce1-Houses Pack": Slifer red, Ra yellow or Obelisk blue) as a port mod.

The pack is made for PPSSPP's texture replacement: duel/ and pre_duel/ hold <hash>--<Variant>.png, ui/ holds
Player{Straight,Diagonal}--<Variant>.png with their hashes in ui/Readme.txt. This writes mods/houses/ with the files
of one variant and a textures.ini in the hash setup of the Tag Force 1 HD pack the files were made for (xxh64,
ignoreAddress), which platform/pc/texmod.c reads. Original = the game's red jacket redrawn in HD.

usage: mod_houses.py PACK_DIR {Ra,Obelisk,Original} [MODS_DIR]     (MODS_DIR default: mods)
       mod_houses.py --check                                          (sanity check, no pack needed)"""
import os, re, shutil, sys, tempfile

UI = {"PlayerStraight": "00000000cd65f46c8fb065c7", "PlayerDiagonal": "00000000a4987f3bf9fca12a"}  # ui/Readme.txt
NAME = re.compile(r"^([0-9a-f]{24})--(\w+)\.png$")


def install(pack, variant, mods="mods"):
    out = os.path.join(mods, "houses")
    if os.path.isdir(out):
        shutil.rmtree(out)
    lines = ["[options]", "version = 1", "hash = xxh64", "ignoreAddress = true", "[hashes]",
             f"# GXTagForce1-Houses Pack, variant {variant} (tools/mod_houses.py)"]
    for sub in ("duel", "pre_duel"):
        for f in sorted(os.listdir(os.path.join(pack, sub))):
            m = NAME.match(f)
            if m and m.group(2) == variant:
                os.makedirs(os.path.join(out, sub), exist_ok=True)
                shutil.copy(os.path.join(pack, sub, f), os.path.join(out, sub, m.group(1) + ".png"))
                lines.append(f"{m.group(1)} = {sub}/{m.group(1)}.png")
    for name, h in UI.items():
        src = os.path.join(pack, "ui", f"{name}--{variant}.png")
        if os.path.exists(src):
            os.makedirs(os.path.join(out, "ui"), exist_ok=True)
            shutil.copy(src, os.path.join(out, "ui", name + ".png"))
            lines.append(f"{h} = ui/{name}.png")
    n = len(lines) - 6
    if not n:
        raise SystemExit(f"no {variant} files in {pack}")
    open(os.path.join(out, "textures.ini"), "w").write("\n".join(lines) + "\n")
    print(f"{out}: {n} textures ({variant})")
    return n


def check():
    with tempfile.TemporaryDirectory() as t:
        pack = os.path.join(t, "pack")
        for sub, f in [("duel", "0000000040201f3565b52b2f--Ra.png"), ("pre_duel", "00000000910e2646422f136f--Ra.png"),
                       ("pre_duel", "00000000910e2646422f136f--Obelisk.png"), ("ui", "PlayerStraight--Ra.png")]:
            os.makedirs(os.path.join(pack, sub), exist_ok=True)
            open(os.path.join(pack, sub, f), "wb").write(b"png")
        assert install(pack, "Ra", os.path.join(t, "mods")) == 3
        ini = open(os.path.join(t, "mods", "houses", "textures.ini")).read()
        assert "00000000910e2646422f136f = pre_duel/00000000910e2646422f136f.png" in ini
        assert "00000000cd65f46c8fb065c7 = ui/PlayerStraight.png" in ini and "Obelisk" not in ini
    print("mod_houses.py: ok")


if __name__ == "__main__":
    if sys.argv[1:] == ["--check"]:
        check()
    elif len(sys.argv) in (3, 4) and sys.argv[2] in ("Ra", "Obelisk", "Original"):
        install(*sys.argv[1:])
    else:
        raise SystemExit(__doc__)
