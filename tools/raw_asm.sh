#!/bin/sh
# raw_asm.sh LO HI: disassembly of the original build/elf/$MODULE.elf (default BOOT) in [LO, HI).
# Use it to check function boundaries the split or Ghidra may have missed.
PSPDEV=${PSPDEV:-$HOME/opt/pspdev}
"$PSPDEV/bin/psp-objdump" -d -mmips:allegrex --start-address="$1" --stop-address="$2" \
  "build/elf/${MODULE:-BOOT}.elf" | grep -P '^\s+[0-9a-f]+:' | cut -f1,3-
