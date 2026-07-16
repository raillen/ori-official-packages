#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"$root/tools/build_linux.sh"
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_JIT="${ORI_USE_JIT:-1}"
export LD_LIBRARY_PATH="$root/lib/x86_64-unknown-linux-gnu:${LD_LIBRARY_PATH:-}"
cd "$root"
"$ORI_BIN" check "$root/lz4/codec.orl"
"$ORI_BIN" check "$root/examples/smoke_lz4.orl"
"$ORI_BIN" check "$root/tests/test_lz4.orl"
out=$("$ORI_BIN" run "$root/examples/smoke_lz4.orl")
echo "$out"
echo "$out" | grep -qx ok
"$ORI_BIN" test "$root/tests/test_lz4.orl"
echo "smoke ok"
