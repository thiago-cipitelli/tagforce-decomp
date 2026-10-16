#!/bin/bash
# Run Metrowerks mwccpsp under wibo. usage: MWCC_BUILD=219 tools/mwcc.sh [mwcc args...]
# Default flags (from TF5 findings, to be confirmed for TF1): -O4,s -sdatathreshold 0
HERE=$(cd "$(dirname "$0")" && pwd)
BUILD=${MWCC_BUILD:-219}
exec "$HERE/ext/wibo" "$HERE/ext/mwccpsp_$BUILD/mwccpsp.exe" "$@"
