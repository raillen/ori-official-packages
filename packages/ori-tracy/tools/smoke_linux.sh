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
if [ ! -f "$libdir/libori_tracy_full.a" ]; then
  echo "missing FULL artifact libori_tracy_full.a" >&2
  exit 1
fi
if ! nm "$libdir/libori_tracy_full.a" | grep -q 'ori_tracy_plot_m'; then
  echo "FULL artifact missing ori_tracy_plot_m" >&2
  exit 1
fi
if ! nm "$libdir/libori_tracy_full.a" | grep -q 'ori_tracy_message'; then
  echo "FULL artifact missing ori_tracy_message" >&2
  exit 1
fi
if ! nm "$libdir/libori_tracy_full.a" | grep -q '___tracy_emit'; then
  echo "FULL artifact missing Tracy client emit symbols" >&2
  exit 1
fi
echo "full artifact ok"

"$ORI_BIN" check "$root/tracy/profiler.orl"
"$ORI_BIN" check "$root/examples/smoke_tracy.orl"
"$ORI_BIN" check "$root/tests/test_tracy.orl"
out=$("$ORI_BIN" run "$root/examples/smoke_tracy.orl")
echo "$out"
echo "$out" | grep -qx ok
"$ORI_BIN" test "$root/tests/test_tracy.orl"
echo "smoke ok"
