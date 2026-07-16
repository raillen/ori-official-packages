#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root"
"$root/tools/build_linux.sh"
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_JIT="${ORI_USE_JIT:-1}"
export LD_LIBRARY_PATH="$root/lib/x86_64-unknown-linux-gnu:${LD_LIBRARY_PATH:-}"

triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
libdir="$root/lib/$triple"
if [ ! -f "$libdir/libori_imguizmo_full.a" ]; then
  echo "missing FULL artifact libori_imguizmo_full.a (sibling ori-imgui required)" >&2
  exit 1
fi
for sym in ori_gizmo_manipulate_rotate ori_gizmo_manipulate_scale \
           ori_curve_sample_m ori_gradient_add_stop_m ori_zoom_span_m; do
  if ! nm "$libdir/libori_imguizmo_full.a" | grep -q "$sym"; then
    echo "FULL artifact missing $sym" >&2
    exit 1
  fi
done
if ! nm "$libdir/libori_imguizmo_full.a" | grep -q 'ImGuizmo'; then
  echo "FULL artifact missing ImGuizmo symbols" >&2
  exit 1
fi
if ! nm "$libdir/libori_imguizmo_full.a" | grep -q 'ImCurveEdit'; then
  echo "FULL artifact missing ImCurveEdit symbols" >&2
  exit 1
fi
if ! nm "$libdir/libori_imguizmo_full.a" | grep -q 'ImGradient'; then
  echo "FULL artifact missing ImGradient symbols" >&2
  exit 1
fi
echo "full artifact ok"

"$ORI_BIN" check "$root/imguizmo/gizmo.orl"
"$ORI_BIN" check "$root/imguizmo/curve.orl"
"$ORI_BIN" check "$root/imguizmo/gradient.orl"
"$ORI_BIN" check "$root/imguizmo/zoom.orl"
"$ORI_BIN" check "$root/examples/smoke_gizmo.orl"
"$ORI_BIN" check "$root/tests/test_gizmo.orl"
"$ORI_BIN" check "$root/tests/test_curve_gradient_zoom.orl"
out=$("$ORI_BIN" run "$root/examples/smoke_gizmo.orl")
echo "$out"
echo "$out" | grep -qx ok
"$ORI_BIN" test "$root/tests/test_gizmo.orl"
"$ORI_BIN" test "$root/tests/test_curve_gradient_zoom.orl"
echo "smoke ok"
