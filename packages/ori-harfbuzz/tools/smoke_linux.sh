#!/usr/bin/env sh
# Build FreeType + HarfBuzz shims and run smoke (JIT).
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ft_root=$(CDPATH= cd -- "$root/../ori-freetype" && pwd)

if [ ! -d "$ft_root" ]; then
  echo "expected sibling ori-freetype at $ft_root" >&2
  exit 1
fi

"$ft_root/tools/build_linux.sh"
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
export LD_LIBRARY_PATH="$root/lib/x86_64-unknown-linux-gnu:$ft_root/lib/x86_64-unknown-linux-gnu:${LD_LIBRARY_PATH:-}"

FONT="${ORI_FT_SMOKE_FONT:-/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf}"
if [ ! -f "$FONT" ]; then
  echo "smoke font missing: $FONT" >&2
  exit 1
fi
export ORI_FT_SMOKE_FONT="$FONT"

echo "== check =="
"$ORI_BIN" check "$root/harfbuzz/shape.orl"
"$ORI_BIN" check "$root/harfbuzz/layout.orl"
"$ORI_BIN" check "$root/examples/smoke_shape.orl"
"$ORI_BIN" check "$root/tests/test_harfbuzz.orl"

echo "== run smoke_shape (JIT) =="
out=$("$ORI_BIN" run "$root/examples/smoke_shape.orl")
echo "$out"
echo "$out" | grep -qx ok

echo "== ori test (AOT) =="
"$ORI_BIN" test "$root/tests/test_harfbuzz.orl"

echo "smoke ok"
