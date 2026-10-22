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

## Status

| Module | What it is | Code matching | Fuzzy | Functions matching |
|---|---|---|---|---|
| `BOOT` | runtime: init, main loop, VFS, scenes, sound, graphics, EHF animation, network, save data | 58.8% | 79.1% | 1777 / 2208 |
| `libduel_eng` | duel engine: rules, card effects, CPU player | 51.3% | 64.5% | 2663 / 3429 |
| `libhtml_view` | smallest gmodule, proof of the build pipeline | 9.8% | 9.8% | 2 / 16 |
| 19 other gmodules | duel presentation, field, story, shop, deck editor, title… | 0% | 0% | 0 / 5386 |
| **total** | | **26.4%** | **34.0%** | **4442 / 11039** |

Numbers from 2026-10-22 (`python3 tools/bytematch.py`); the badges above are live. The whole game
rebuilds byte-identical today: the parts without C are kept as assembly. Per-file progress and the
largest ranges still without C are in [docs/progress.md](docs/progress.md).

| # | Phase | State |
|---|---|---|
| 0 | Analysis setup (ISO extraction, PRX → ELF, Ghidra) | done |
| 1 | Map of the binary (`docs/architecture.md`, `docs/gmodules.md`, `symbols/`) | done |
| 2 | Data formats (EHP, GIM, CIP, card database, sound, scripts) | in progress |
| 3 | Reproducible matching build (splat + mwccpsp + Makefile) | done |
| 4 | Decompile the game code | in progress |
| 5 | Platform layer (`platform/psp`, `platform/pc`) | to do |
| 6 | Playable PC port | to do |
| 7 | Parity and polish (video, Atrac3+ audio, saves, input) | to do |

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

## Getting started

You need Linux x86_64 (WSL works), Python 3.10+, `curl`, and your own dump of the game.

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
| `tools/` | Python and shell tools: ISO/PRX conversion, data formats (EHP, GIM, CIP), build, matching, progress |
| `docs/` | technical docs (in Portuguese) |
| `asm/`, `assets/`, `build/`, `extracted/`, `tools/ext/` | generated or downloaded, gitignored |

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
