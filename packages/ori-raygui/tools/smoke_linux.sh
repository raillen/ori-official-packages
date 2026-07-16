#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
"$root/tools/build_linux.sh"
"$ORI_BIN" check "$root/raygui/ui.orl"
"$ORI_BIN" check "$root/examples/hello_raygui.orl"
out=$(mktemp)
"$ORI_BIN" compile "$root/examples/hello_raygui.orl" --out "$out"
set +e
timeout 3 "$out" >/dev/null 2>&1
code=$?
set -e
rm -f "$out"
if [ "$code" -eq 0 ] || [ "$code" -eq 124 ]; then
  echo "smoke_raygui: all green (exit $code)"
else
  echo "smoke_raygui failed exit $code" >&2
  exit 1
fi
