#!/usr/bin/env sh
# Headless smoke: knob / toggle / spinner / spectrum (no GUI).
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root"
"$root/tools/build_linux.sh"

ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
# Path-dep imgui ships static .a only (no cdylib). Headless widgets do not call
# imgui native, but package resolution still lists imgui native_libs — use AOT.
export ORI_USE_AOT="${ORI_USE_AOT:-1}"
unset ORI_USE_JIT || true

echo "== check modules + smoke + tests =="
"$ORI_BIN" check "$root/widgets/knob.orl"
"$ORI_BIN" check "$root/widgets/toggle.orl"
"$ORI_BIN" check "$root/widgets/spinner.orl"
"$ORI_BIN" check "$root/widgets/spectrum.orl"
"$ORI_BIN" check "$root/examples/smoke_widgets.orl"
"$ORI_BIN" check "$root/tests/test_widgets.orl"

echo "== run smoke (headless AOT) =="
out=$("$ORI_BIN" run "$root/examples/smoke_widgets.orl")
echo "$out"
echo "$out" | grep -qx ok

echo "== ori test =="
"$ORI_BIN" test "$root/tests/test_widgets.orl"

echo "smoke ok"
