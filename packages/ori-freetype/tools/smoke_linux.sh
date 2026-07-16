#!/usr/bin/env sh
# Build + smoke FreeType package (Linux). Prefers JIT (shared shim + system FreeType).
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"$root/tools/build_linux.sh"

ORI_BIN="${ORI_BIN:-}"
if [ -z "$ORI_BIN" ]; then
  if command -v ori >/dev/null 2>&1; then
    ORI_BIN=$(command -v ori)
  else
    echo "set ORI_BIN" >&2
    exit 2
  fi
fi

export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_JIT="${ORI_USE_JIT:-1}"
export LD_LIBRARY_PATH="$root/lib/x86_64-unknown-linux-gnu:${LD_LIBRARY_PATH:-}"

FONT="${ORI_FT_SMOKE_FONT:-/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf}"
if [ ! -f "$FONT" ]; then
  echo "smoke font missing: $FONT (set ORI_FT_SMOKE_FONT)" >&2
  exit 1
fi
export ORI_FT_SMOKE_FONT="$FONT"

echo "== check =="
"$ORI_BIN" check "$root/freetype/face.orl"
"$ORI_BIN" check "$root/freetype/text.orl"
"$ORI_BIN" check "$root/freetype/atlas.orl"
"$ORI_BIN" check "$root/examples/smoke_face.orl"
"$ORI_BIN" check "$root/tests/test_freetype.orl"

echo "== run smoke_face =="
out=$("$ORI_BIN" run "$root/examples/smoke_face.orl")
echo "$out"
echo "$out" | grep -qx ok

echo "== ori test =="
"$ORI_BIN" test "$root/tests/test_freetype.orl"

echo "smoke ok"
