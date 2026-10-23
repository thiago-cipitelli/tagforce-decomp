# Contributing

Thanks for helping. This guide covers what a contribution looks like, how to find work, the
workflow from asm to matching C, and the checks a change must pass. Set up the toolchain and the
build first ([README → Getting started](README.md#getting-started)); you are ready when
`make check-all` prints `MATCH` for all 23 modules.

## Ground rules

- **Never commit anything from the game**: the ISO, `extracted/`, `build/`, `asm/`, `assets/` or any
  file derived from game data. They are gitignored; keep it that way. Tools that read game data are
  fine, their output is not.
- **Every change keeps the build byte-identical.** `make MODULE=<module> verify` must print
  `ALL MATCH` before you open a pull request.
- Don't guess about behaviour. If you describe what the game does (in a comment or in `docs/`), say
  whether it is observed in the binary, inferred, or unknown. Open questions go in the PR description.
- Code identifiers are in English. The docs in `docs/` and the existing commit history are in
  Portuguese; English is fine for your commits, comments and PRs.

## Finding work

```sh
MODULE=libduel_eng python3 tools/list_funcs.py --gaps          # largest ranges without C
MODULE=libduel_eng python3 tools/list_funcs.py 0x08889BE4 0x0888B000   # functions in a range
```

[docs/progress.md](docs/progress.md) lists every C file and the open ranges. The current front lines:

- `libduel_eng`: about 300 functions kept in asm with a C draft under `#ifdef NON_MATCHING`, many only a few words off; `docs/build.md` lists the source shapes that fixed similar ones. Good first work.
- `libduel_eng`: about 30 ranges still without C, mostly single large functions (`0x08839360`-`0x0883F0DC`, `0x088CC8EC`-`0x088D13CC`, the CPU step machines between `0x08832B54` and `0x08864DF0`); `list_funcs.py --gaps` lists them.
- 9 gmodules (`libduel_draw`, `libfield`, `libstory`, `libtitle`, `libdeck`, …) have not been started: there is
  room for someone to take a whole module.
- `BOOT` has C for its whole `.text`; about 400 functions remain in `@asm` with a C draft under
  `NON_MATCHING`. The GCC-compiled SDK code is written as mwcc `asm` functions with portable C under
  `#else`; the ones left in `@asm` load data with `lui`/`%lo`, which the inline assembler can't express.

Before starting a range, open an issue titled `claim: <module> 0xLO-0xHI` so two people don't work on
the same functions. Close it with the PR.

## Workflow

All tools take the module from the `MODULE` environment variable (default `BOOT`).

1. **Read the function.**
   - `tools/func_asm.sh func_XXXXXXXX`: the asm from the split. Functions keep their splat name
     `func_XXXXXXXX` until you name them and re-run `make`.
   - `tools/decomp_view.py LO HI`: Ghidra's pseudocode with the names from `symbols/` (optional; needs
     the Ghidra export, see below). It is a starting point only: it gets bitfields, signedness and
     loops wrong. Write the C from the asm.
   - `tools/raw_asm.sh LO HI`: the original ELF, useful when a function boundary looks wrong.

2. **Write the C** in `src/<module>/<file>.c`. One file covers one contiguous address range of the
   original, with its functions in address order. The header says what the file covers:

   ```c
   // @range 0x08887618-0x08889BE4   address range of the functions in this file
   // @data  0x089117DC-0x08911828   .data this file emits (literals, switch jump tables); optional
   // @asm   0x08887618-0x088876B0   a function in the range that stays in asm; optional, repeatable
   /* What this file is about, in one sentence. */
   #include "duel.h"
   ```

   Extend an existing file if your range continues it; start a new file for a new subsystem. Shared
   structs go in `include/` (`include/duel.h` has the duel state: `Duel`, `DuelPlayer`, `DuelSlot`,
   `DuelCard`, `DuelAct`). Declare the callees at the top of the file with the prototype the original
   used; a wrong parameter type shows up as a missing or extra `andi`/`seh` before the call.

3. **Name things** in `symbols/<module>.txt`, above the `// --- generated imports below` line:

   ```sh
   cat > /tmp/names.txt <<'EOF'
   0x08887618 duel_08887618 type:func matched src/libduel_eng/effects_x.c
   0x088D1F1C duel_088D1F1C type:func inferred
   EOF
   MODULE=libduel_eng python3 tools/sym_update.py /tmp/names.txt
   ```

   Use a descriptive name when you know what a function does, `<prefix>_<ADDR>` when you don't. The
   comment says `matched src/...` for C that matches, `asm (@asm in src/..., C under NON_MATCHING)` for
   C that doesn't yet, `inferred` for names of functions that are still asm.

4. **Match it.**
   - `python3 tools/check_file.py <module> src/<module>/<file>.c` compiles the file and compares every
     function in the `@range` (`OK` / `BAD ... N words differ`).
   - `tools/func_diff.sh src/<module>/<file>.c <function>` shows original | ours side by side.
     Relocated operands (`lui`/`addiu` of globals, `jal` targets) appear as `0` on the right; they are
     not real differences.
   - `python3 tools/perm_locals.py src/<module>/<file>.c <function> --apply` tries every order of the
     function's local declarations, which decides register allocation, and keeps the best one.
     `tools/perm_decls.py` does the same for template-based variants of whole expressions.
   - [docs/build.md](docs/build.md), section *"O que o mwccpsp faz com o fonte"*, collects the source
     shapes that produce each code pattern (early return vs. nested `if`, `switch` case order, `MIN`/`MAX`,
     bitfields vs. masks, ...). Read it before spending an hour on one function.

5. **When a function will not match**, don't block on it. Add `// @asm LO-HI` for it to the header and
   keep your C under `#ifdef NON_MATCHING /* what differs */`. Functional C is still useful for the
   port, and someone can finish the match later.

6. **Verify** before committing:

   ```sh
   make MODULE=<module> verify   # regenerates config/, re-splits, checks the module, refreshes imports,
                                 # then checks all 23 modules: must print ALL MATCH
   ```

   `check_file.py` ignores relocations, so a wrong struct offset or global only shows up here. When it
   does, `python3 tools/diff_words.py build/elf/<m>.elf build/<m>.rebuilt.elf .text` points to the
   first differing word.

7. **Update [docs/progress.md](docs/progress.md)** with the line for your file (range, functions
   matching, in asm, one line on what it contains).

## Commits and pull requests

- Small commits, one file or one range each. Include `config/<module>.yaml` and `symbols/` changes
  together with the `.c`.
- Message format used in the history:
  `Fase 4: effects_x.c (0x08887618-0x08889BE4, 13 funções matching, 7 em asm): handlers de efeito; libduel_eng MATCH, check-all MATCH`
  (Portuguese or English).
- In the PR, say which functions are still in asm and why, and anything you found out about the
  game's behaviour.

## Optional: Ghidra export and REA

`tools/decomp_view.py` reads `build/decomp/<module>/`, produced by a headless Ghidra run with the
[ghidra-allegrex](https://github.com/kotcrab/ghidra-allegrex) extension:

```sh
$GHIDRA/support/analyzeHeadless build/ghidra tf -import build/elf/libduel_eng.elf \
  -scriptPath tools/ghidra -postScript ExportDecomp.java symbols/libduel_eng.txt build/decomp/libduel_eng \
  -deleteProject
```

The maintainer also uses the [REA](https://github.com/morluto/rea) MCP server on top of Ghidra for
interactive analysis ([docs/build.md](docs/build.md)). Neither is required to contribute.

## AI agents

Agents (Claude Code, Codex, ...) read [AGENTS.md](AGENTS.md): the project plan, the facts about the
binary that don't need re-investigating, and the conventions above. A PR made with an agent follows
the same rules: byte-identical build, no game data, and you review what it wrote.
