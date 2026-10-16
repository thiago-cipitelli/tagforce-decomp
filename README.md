# Yu-Gi-Oh! GX Tag Force (PSP, ULUS-10136) decompilation

Goal: reconstruct the game's C source and build a native PC port.
Modelled on [ygo-tf-5-decomp](https://github.com/angelof-exe/ygo-tf-5-decomp).

Binary facts: BOOT.BIN and `USRDIR/gmodule/*.prx` are unencrypted relocatable
PSP ELFs built with Metrowerks `MW MIPS C Compiler (2.4.1.01)`. Game data is in
EHP archives.

## Setup

```sh
pip install pycdlib
python3 tools/extract_iso.py "Yu-Gi-Oh! GX - Tag Force (USA).iso" extracted
mkdir -p build/elf
python3 tools/prx2elf.py extracted/PSP_GAME/SYSDIR/BOOT.BIN build/elf/BOOT.elf
for p in extracted/PSP_GAME/USRDIR/gmodule/*.prx; do python3 tools/prx2elf.py "$p" "build/elf/$(basename "$p" .prx).elf"; done
```

`prx2elf.py` applies the PRX relocations at base `0x08804000` and emits a
static `ET_EXEC` ELF (plus a `.MIPS.abiflags` record) so Ghidra, splat and the
[REA](https://github.com/morluto/rea) PSP profile accept it.

## Analysis via REA (Ghidra 12.1.3 + ghidra-allegrex v21.4)

The REA MCP registration in `~/.claude.json` needs `GHIDRA_INSTALL_DIR` set to
the Ghidra install with the ghidra-allegrex extension under `Ghidra/Extensions/`.
Open `build/elf/BOOT.elf` with `open_binary` and `provider_id: "ghidra"`.
