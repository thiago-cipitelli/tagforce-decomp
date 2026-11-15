#!/bin/bash
# Run Metrowerks mwccpsp under wibo. usage: MWCC_BUILD=219 tools/mwcc.sh [mwcc args...]
# Default flags (from TF5 findings, to be confirmed for TF1): -O4,s -sdatathreshold 0
# A source whose header has `// @cc gcc` (SDK code that GCC compiled) goes to tools/psp_gcc.sh instead, so every
# tool that compiles through this script follows the marker.
HERE=$(cd "$(dirname "$0")" && pwd)
BUILD=${MWCC_BUILD:-219}
for a in "$@"; do
  case "$a" in *.c) head -20 "$a" 2>/dev/null | grep -q '^// @cc gcc' && exec "$HERE/psp_gcc.sh" "$@";; esac
done
exec "$HERE/ext/wibo" "$HERE/ext/mwccpsp_$BUILD/mwccpsp.exe" "$@"
