#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
"$root/tools/build_linux.sh"
"$ORI_BIN" check "$root/imgui/ui.orl"
"$ORI_BIN" check "$root/imgui/file_browser.orl"
"$ORI_BIN" check "$root/imgui/inspector.orl"
"$ORI_BIN" check "$root/imgui/plot.orl"
"$ORI_BIN" check "$root/imgui/nodes.orl"
"$ORI_BIN" check "$root/imgui/curves.orl"
"$ORI_BIN" check "$root/imgui/timeline.orl"
"$ORI_BIN" check "$root/imgui/test_harness.orl"
"$ORI_BIN" check "$root/examples/demo.orl"
"$ORI_BIN" check "$root/examples/tier1_demo.orl"
"$ORI_BIN" test "$root/tests/test_timeline.orl"
"$ORI_BIN" test "$root/tests/test_curves_timeline.orl"
"$ORI_BIN" test "$root/tests/test_context_image.orl"
"$ORI_BIN" test "$root/tests/test_test_harness.orl"
out=$(mktemp)
"$ORI_BIN" compile "$root/examples/demo.orl" --out "$out"
set +e
timeout 3 "$out" >/tmp/ori_imgui_smoke.log 2>&1
code=$?
set -e
rm -f "$out"
if [ "$code" -ne 0 ] && [ "$code" -ne 124 ]; then
  echo "smoke_imgui demo failed exit $code" >&2
  tail -40 /tmp/ori_imgui_smoke.log >&2 || true
  exit 1
fi
out2=$(mktemp)
"$ORI_BIN" compile "$root/examples/tier1_demo.orl" --out "$out2"
set +e
timeout 3 "$out2" >/tmp/ori_imgui_tier1_smoke.log 2>&1
code2=$?
set -e
rm -f "$out2"
if [ "$code2" -eq 0 ] || [ "$code2" -eq 124 ]; then
  echo "smoke_imgui: all green (demo=$code tier1=$code2)"
else
  echo "smoke_imgui tier1 failed exit $code2" >&2
  tail -40 /tmp/ori_imgui_tier1_smoke.log >&2 || true
  exit 1
fi
