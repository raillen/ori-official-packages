#!/usr/bin/env sh
# Headless smoke: toast / search / hotkey / palette / metrics (no GUI).
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root"
"$root/tools/build_linux.sh"

ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
# Path-dep imgui ships static .a only (no cdylib). Headless extras do not call
# imgui native, but package resolution still lists imgui native_libs — use AOT.
export ORI_USE_AOT="${ORI_USE_AOT:-1}"
unset ORI_USE_JIT || true

echo "== check modules + smoke + tests =="
"$ORI_BIN" check "$root/imgui_extras/notify.orl"
"$ORI_BIN" check "$root/imgui_extras/search.orl"
"$ORI_BIN" check "$root/imgui_extras/hotkey.orl"
"$ORI_BIN" check "$root/imgui_extras/command_palette.orl"
"$ORI_BIN" check "$root/imgui_extras/metrics.orl"
"$ORI_BIN" check "$root/examples/smoke_imgui_extras.orl"
"$ORI_BIN" check "$root/tests/test_imgui_extras.orl"

echo "== run smoke (headless AOT) =="
out=$("$ORI_BIN" run "$root/examples/smoke_imgui_extras.orl")
echo "$out"
echo "$out" | grep -qx ok

echo "== ori test =="
"$ORI_BIN" test "$root/tests/test_imgui_extras.orl"

echo "smoke ok"
