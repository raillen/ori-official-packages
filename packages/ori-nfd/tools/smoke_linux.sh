#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root"
"$root/tools/build_linux.sh"
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_JIT="${ORI_USE_JIT:-1}"
# Never hang on headless CI: skip real dialogs if any code path calls them.
export NFD_SMOKE_SKIP_UI="${NFD_SMOKE_SKIP_UI:-1}"
export LD_LIBRARY_PATH="$root/lib/x86_64-unknown-linux-gnu:${LD_LIBRARY_PATH:-}"
"$ORI_BIN" check "$root/nfd/dialog.orl"
"$ORI_BIN" check "$root/examples/smoke_nfd.orl"
"$ORI_BIN" check "$root/tests/test_nfd.orl"
out=$("$ORI_BIN" run "$root/examples/smoke_nfd.orl")
echo "$out"
echo "$out" | grep -qx ok
"$ORI_BIN" test "$root/tests/test_nfd.orl"
echo "smoke ok"
