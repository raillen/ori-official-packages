#!/usr/bin/env sh
# Headless smoke: buffer + language stub only (no GUI / no FULL C++ client).
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root"
"$root/tools/build_linux.sh"

ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
# Path-dep imgui ships static .a only (no cdylib). Headless buffer does not call
# imgui, but package resolution still lists imgui native_libs — use AOT for run
# so JIT does not require libori_imgui.so. `ori test` is always AOT.
export ORI_USE_AOT="${ORI_USE_AOT:-1}"
unset ORI_USE_JIT || true

echo "== check buffer + lang + smoke + tests =="
"$ORI_BIN" check "$root/imtextedit/buffer.orl"
"$ORI_BIN" check "$root/imtextedit/lang.orl"
"$ORI_BIN" check "$root/examples/smoke_imtextedit.orl"
"$ORI_BIN" check "$root/tests/test_imtextedit.orl"

# Draw module typecheck when sibling imgui is present (does not open a window).
if [ -f "$root/../ori-imgui/imgui/ui.orl" ]; then
  echo "== check draw (path-dep imgui) =="
  "$ORI_BIN" check "$root/imtextedit/draw.orl"
fi

echo "== run smoke (headless AOT) =="
out=$("$ORI_BIN" run "$root/examples/smoke_imtextedit.orl")
echo "$out"
echo "$out" | grep -qx ok

echo "== ori test =="
"$ORI_BIN" test "$root/tests/test_imtextedit.orl"

echo "smoke ok"
