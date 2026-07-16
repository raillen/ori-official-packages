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
if [ ! -f "$libdir/libori_implot3d_full.a" ]; then
  echo "missing FULL artifact libori_implot3d_full.a (sibling ori-imgui required)" >&2
  exit 1
fi
if ! nm "$libdir/libori_implot3d_full.a" | grep -q 'ori_implot3d_plot_scatter'; then
  echo "FULL artifact missing scatter" >&2
  exit 1
fi
if ! nm "$libdir/libori_implot3d_full.a" | grep -q 'ori_implot3d_plot_line'; then
  echo "FULL artifact missing line" >&2
  exit 1
fi
if ! nm "$libdir/libori_implot3d_full.a" | grep -q 'ImPlot3D'; then
  echo "FULL artifact missing ImPlot3D vendor symbols" >&2
  exit 1
fi
echo "full artifact ok"

"$ORI_BIN" check "$root/implot3d/charts.orl"
"$ORI_BIN" check "$root/examples/smoke_implot3d.orl"
"$ORI_BIN" check "$root/tests/test_implot3d.orl"
out=$("$ORI_BIN" run "$root/examples/smoke_implot3d.orl")
echo "$out"
echo "$out" | grep -qx ok
"$ORI_BIN" test "$root/tests/test_implot3d.orl"
echo "smoke ok"
