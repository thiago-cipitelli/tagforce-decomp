# Yu-Gi-Oh! GX Tag Force decompilation

[![Code](https://bytematch.dev/thiago-cipitelli/tagforce-decomp.svg?label=Code&measure=matched_code_percent)](https://bytematch.dev/thiago-cipitelli/tagforce-decomp)
[![Fuzzy](https://bytematch.dev/thiago-cipitelli/tagforce-decomp.svg?label=Fuzzy&measure=fuzzy_match_percent)](https://bytematch.dev/thiago-cipitelli/tagforce-decomp)
[![Functions](https://bytematch.dev/thiago-cipitelli/tagforce-decomp.svg?label=Functions&measure=matched_functions)](https://bytematch.dev/thiago-cipitelli/tagforce-decomp)

*Yu-Gi-Oh! GX Tag Force* (PSP, Konami 2006, ULUS-10136) decompiled back into C, so that it can run
natively on PC with no emulator, reading the assets from your own copy of the game.

The C is checked by recompiling it with the original compiler (Metrowerks `mwccpsp`) and comparing
the result byte for byte against the game. Matching is how we verify the code, not the end goal: the
target is readable, portable C with the game logic (duel engine, story, menus) separated from the PSP
platform layer. The project is modelled on [ygo-tf-5-decomp](https://github.com/angelof-exe/ygo-tf-5-decomp).

## Playing on PC

The port runs the game natively on Linux (x86_64), with no emulator, from your own copy of the disc.

> **Work in progress.** The game boots, plays the intro movies, and runs the menus and story mode with music
> and sound effects. Not every part of the game has been played through on the port yet; if something breaks,
> please open an issue with the steps.

### What you need

- Linux x86_64. The game code is 32-bit, so you need a `gcc` that builds 32-bit programs and the 32-bit X11,
  OpenGL, FreeType, ALSA and libpng libraries.
- Python 3.10+, `ffmpeg` (movies and music), and a TTF font (Liberation Sans or DejaVu Sans).
- Your own dump of the North American disc (see [Supported disc](#supported-disc)).

On Arch Linux (and CachyOS, Manjaro, EndeavourOS), with the `multilib` repository enabled:

```sh
sudo pacman -S gcc lib32-gcc-libs lib32-glibc lib32-libx11 lib32-mesa lib32-freetype2 lib32-alsa-lib \
  lib32-libpng ffmpeg ttf-liberation python
```

On Debian/Ubuntu (package names may differ between releases):

```sh
sudo dpkg --add-architecture i386 && sudo apt update
sudo apt install gcc-multilib libx11-dev:i386 libgl-dev:i386 libfreetype-dev:i386 libasound2-dev:i386 \
  libpng-dev:i386 ffmpeg fonts-liberation python3-venv
```

### Build and run

```sh
git clone https://github.com/thiago-cipitelli/tagforce-decomp.git
cd tagforce-decomp

# 1. extract your disc (put the ISO in the repo root; it is gitignored) and convert the game modules
python3 -m venv .venv && .venv/bin/pip install pycdlib
.venv/bin/python tools/extract_iso.py "Yu-Gi-Oh! GX - Tag Force (USA).iso" extracted
mkdir -p build/elf
python3 tools/prx2elf.py extracted/PSP_GAME/SYSDIR/BOOT.BIN build/elf/BOOT.elf
for p in extracted/PSP_GAME/USRDIR/gmodule/*.prx; do
  python3 tools/prx2elf.py "$p" "build/elf/$(basename "$p" .prx).elf"
done

# 2. build the game (a few minutes the first time): prints "0 undefined, 0 duplicate"
make host-link

# 3. play
TAGFORCE_ROOT=extracted build/host/tagforce
```

You don't need the matching toolchain (`tools/setup_toolchain.sh`) just to play. After a `git pull`, run
`make host-link` again.

### Controls

| PSP | Keyboard |
|---|---|
| D-pad | arrow keys |
| ✕ (confirm) | `Z` |
| ○ (cancel) | `X` |
| △ | `S` |
| □ | `A` |
| L / R | `Q` / `W` |
| START / SELECT | `Enter` / `Backspace` |

When the game asks for your name, type it on the keyboard and press `Enter` (`Esc` cancels).

### Saves and options

Saves go to `~/.local/share/tagforce/ms0` (the PSP memory stick). Environment variables:

| Variable | Default | What it does |
|---|---|---|
| `TAGFORCE_ROOT` | current directory | folder with the extracted disc (`PSP_GAME/` inside) |
| `TAGFORCE_SAVE` | `~/.local/share/tagforce/ms0` | save folder |
| `TAGFORCE_SCALE` | `2` | window size per PSP pixel (`3` = 1440×816) |
| `TAGFORCE_MODS` | `mods` | folder with the [mods](#mods-optional) |
| `TAGFORCE_AUDIO` | default ALSA device | ALSA device name, or `0` for no sound |
| `TAGFORCE_TTF` | Liberation Sans, then DejaVu Sans | font file for the in-game text |

### Mods (optional)

Mods change only what you see, never the game code, and are off unless you install them. Each mod is a folder in
`mods/` (gitignored) holding a texture pack in [PPSSPP's format](https://www.ppsspp.org/docs/reference/texture-replacement/)
(`textures.ini` plus PNGs), so the packs made for the emulator work as they are:

```
mods/
  mods.txt          # optional: enabled mods, one per line, first = highest priority
  houses/           # protagonist outfit (see below)
    textures.ini
    ...
  hd/               # e.g. the ULUS10136 folder of a Tag Force 1 HD texture pack
```

Without `mods.txt` every folder in `mods/` is enabled, in alphabetical order (the first one wins when two mods
replace the same texture). To turn a mod off, remove it from `mods.txt` or move its folder out of `mods/`.
Packs with `hash = quick` in their `textures.ini` are not supported yet (`xxh32` and `xxh64` are).

**Protagonist outfit** (the *GXTagForce1-Houses Pack*: Ra Yellow, Obelisk Blue, or the original Slifer Red
redrawn in HD). Point the tool at the unzipped pack and pick the house:

```sh
python3 tools/mod_houses.py ~/Downloads/"GXTagForce1-Houses Pack" Ra    # or Obelisk, or Original
```

It writes `mods/houses/`. Run it again with another house to switch.

**HD texture packs** for Tag Force 1: copy (or symlink) the pack's `ULUS10136` folder into `mods/` under any
name. If you also use the houses mod, list it first in `mods/mods.txt` so it wins over the pack's own portraits:

```
houses
hd
```

`TAGFORCE_TEXMOD_LOG=1` prints each replaced texture and the name of each texture with no replacement (the name
a new pack would use).

## Status

| Module | What it is | Code matching | Functions matching |
|---|---|---|---|
| `BOOT` | runtime: init, main loop, VFS, scenes, sound, graphics, EHF animation, network, save data | 97.7% | 2177 / 2205 |
| `libduel_eng` | duel engine: rules, card effects, CPU player | 98.8% | 3424 / 3429 |
| `libduel_draw` | duel presentation | 98.6% | 1606 / 1609 |
| `libstory` | story mode | 99.8% | 675 / 676 |
| the other 19 gmodules | title, field map, shop, deck editor, duel scene, tutorials, card album, … | 100% | 3117 / 3117 |
| **total** | | **99.0%** | **10999 / 11036** |

Numbers from 2026-11-18 (`python3 tools/bytematch.py`); the badges above are live. Every module rebuilds
byte-identical; the few functions that don't match yet are kept as assembly, with their C under
`NON_MATCHING` (that C is what the PC port uses). Per-file progress is in [docs/progress.md](docs/progress.md).

| # | Phase | State |
|---|---|---|
| 0 | Analysis setup (ISO extraction, PRX → ELF, Ghidra) | done |
| 1 | Map of the binary (`docs/architecture.md`, `docs/gmodules.md`, `symbols/`) | done |
| 2 | Data formats (EHP, GIM, CIP, card database, sound, scripts) | in progress |
| 3 | Reproducible matching build (splat + mwccpsp + Makefile) | done |
| 4 | Decompile the game code | almost done (99.0%) |
| 5 | Platform layer (`platform/pc`) | in progress |
| 6 | Playable PC port | in progress |
| 7 | Parity and polish (video, Atrac3+ audio, saves, input) | in progress |

## How the game is built

- `PSP_GAME/SYSDIR/BOOT.BIN` is the runtime. It loads modules on demand from
  `USRDIR/gmodule/*.prx` (title, field, story, shop, deck, `duel_eng`, `duel_draw`, `duel_mgr`, …),
  which export their functions by name.
- All of them are unencrypted relocatable PSP ELFs built with `MW MIPS C Compiler (2.4.1.01)`.
  `tools/prx2elf.py` applies the relocations at `0x08804000` so the tools see a static ELF.
- Game data lives in EHP archives, card images in CIP files, the card database in
  `duelsys/cardinfo_*.ehp`, audio in `psp_snddat.bin`.

More in [docs/architecture.md](docs/architecture.md) and [docs/gmodules.md](docs/gmodules.md).

## Supported disc

Only the North American release. Check your dump before starting:

| File | Size | SHA-1 | MD5 |
|---|---|---|---|
| `Yu-Gi-Oh! GX - Tag Force (USA).iso` (ULUS-10136) | 701693952 | `2b0ec3ea6bc453c806e4633aeea4ae00287959d2` | `9c84d00f5a4c19c38c38b45ac1be3cff` |
| `PSP_GAME/SYSDIR/BOOT.BIN` | | `88e20f66bffbd76fc1a0fde8533e99577cd1288d` | `929102ff4b1e3a2ff83b025bc100eed0` |

## Getting started (decompilation)

This is for working on the decompilation (to just play, see [Playing on PC](#playing-on-pc)). You need
Linux x86_64 (WSL works), Python 3.10+, `curl`, and your own dump of the game.

```sh
git clone https://github.com/thiago-cipitelli/tagforce-decomp.git
cd tagforce-decomp

# 1. toolchain: wibo, mwccpsp 3.0.1 build 219, objdiff-cli in tools/ext/, pspdev in ~/opt/pspdev, .venv
tools/setup_toolchain.sh

# 2. your disc: put the ISO in the repo root (it is gitignored), extract it and convert the modules
sha1sum "Yu-Gi-Oh! GX - Tag Force (USA).iso"
.venv/bin/python tools/extract_iso.py "Yu-Gi-Oh! GX - Tag Force (USA).iso" extracted
mkdir -p build/elf
python3 tools/prx2elf.py extracted/PSP_GAME/SYSDIR/BOOT.BIN build/elf/BOOT.elf
for p in extracted/PSP_GAME/USRDIR/gmodule/*.prx; do
  python3 tools/prx2elf.py "$p" "build/elf/$(basename "$p" .prx).elf"
done

# 3. build and compare against the original
make MODULE=BOOT check      # split with splat, compile src/BOOT, link, compare: prints MATCH
make check-all              # every module: 23 lines ending in MATCH
```

`make MODULE=<module>` alone builds without comparing; `make MODULE=<module> verify` is the full gate
to run before a commit (see [CONTRIBUTING.md](CONTRIBUTING.md)). The pipeline is described in
[docs/build.md](docs/build.md).

## Decompiling a function

1. Pick a range with no C: `MODULE=libduel_eng python3 tools/list_funcs.py --gaps`.
2. Read the asm (`tools/func_asm.sh func_XXXXXXXX`) and, if you have the Ghidra export, the
   pseudocode (`tools/decomp_view.py LO HI`).
3. Write the C in `src/<module>/<file>.c` with a `// @range LO-HI` header.
4. Name the functions in `symbols/<module>.txt` (`tools/sym_update.py`).
5. Compile and compare the file: `python3 tools/check_file.py <module> src/<module>/<file>.c`.
6. Look at what differs: `tools/func_diff.sh src/<module>/<file>.c <function>`.
7. If a function will not match, keep it as asm (`// @asm LO-HI`) with its C under `#ifdef NON_MATCHING`.
8. `make MODULE=<module> verify` must print `ALL MATCH`.

The details, and the source shapes that make `mwccpsp` produce the original code, are in
[CONTRIBUTING.md](CONTRIBUTING.md) and [docs/build.md](docs/build.md).

## Repository layout

| Path | Contents |
|---|---|
| `src/<module>/` | reconstructed C, one file per contiguous range of the original |
| `include/` | headers: PSP SDK (`psp.h`), runtime subsystems, duel state (`duel.h`) |
| `symbols/<module>.txt` | names of functions and data (splat format); the source of truth for naming |
| `config/<module>.yaml` | splat configs, generated by `tools/splat_config.py` from the `.c` headers |
| `platform/pc/` | the PC port: PSP firmware calls on Linux (window, GE → OpenGL, audio, files, texture mods) |
| `tools/` | Python and shell tools: ISO/PRX conversion, data formats (EHP, GIM, CIP), build, matching, progress |
| `docs/` | technical docs (in Portuguese) |
| `asm/`, `assets/`, `build/`, `extracted/`, `mods/`, `tools/ext/` | generated, downloaded or your own, gitignored |

## Documentation

The technical docs are written in Brazilian Portuguese.

| File | Contents |
|---|---|
| [docs/architecture.md](docs/architecture.md) | the BOOT runtime: init, main loop, scenes, module loader, VFS, graphics |
| [docs/gmodules.md](docs/gmodules.md) | each gmodule: entry point, state machine, data, dependencies |
| [docs/formats.md](docs/formats.md) | data formats: EHP, GIM, CIP/CPM, card database |
| [docs/build.md](docs/build.md) | matching build pipeline and what `mwccpsp` does with each source shape |
| [docs/progress.md](docs/progress.md) | per-file progress and ranges still without C |
| [AGENTS.md](AGENTS.md) | project plan, decisions and conventions (also the instructions for AI agents) |

## Contributing

Contributions are welcome, from a single function to a whole module. Read
[CONTRIBUTING.md](CONTRIBUTING.md) first. AI coding agents must follow [AGENTS.md](AGENTS.md).

## Credits and legal

Built on [splat](https://github.com/ethteck/splat), [spimdisasm](https://github.com/Decompollaborate/spimdisasm),
[wibo](https://github.com/decompals/wibo), the [decomp.me](https://decomp.me) compiler collection,
[objdiff](https://github.com/encounter/objdiff), [pspdev](https://github.com/pspdev/pspdev),
[Ghidra](https://github.com/NationalSecurityAgency/ghidra) with
[ghidra-allegrex](https://github.com/kotcrab/ghidra-allegrex), and
[ygo-tf-5-decomp](https://github.com/angelof-exe/ygo-tf-5-decomp).

This is an unofficial fan project, not affiliated with or endorsed by Konami. Yu-Gi-Oh! is a trademark
of its respective owners. The repository contains no game code, assets or binaries: you need your own
legally obtained copy of the game. The code and tools here are licensed under the
[GNU General Public License v3.0](LICENSE).
