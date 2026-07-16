#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"$root/tools/build_linux.sh"
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_JIT="${ORI_USE_JIT:-1}"
export LD_LIBRARY_PATH="$root/lib/x86_64-unknown-linux-gnu:${LD_LIBRARY_PATH:-}"
# Fixture paths in smoke/tests are relative to package root.
cd "$root"
"$ORI_BIN" check "$root/cgltf/loader.orl"
"$ORI_BIN" check "$root/examples/smoke_cgltf.orl"
"$ORI_BIN" check "$root/tests/test_cgltf.orl"
out=$("$ORI_BIN" run "$root/examples/smoke_cgltf.orl")
echo "$out"
echo "$out" | grep -qx ok
"$ORI_BIN" test "$root/tests/test_cgltf.orl"
echo "smoke ok"
