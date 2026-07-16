#!/usr/bin/env sh
# Pure Ori package — no native shim. Sibling ori-imgui is required for draw module only.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"

imgui_root="$root/../ori-imgui"
if [ -d "$imgui_root" ]; then
  echo "sibling ori-imgui present (draw path available)"
else
  echo "warn: sibling ori-imgui missing at $imgui_root (draw module needs it)" >&2
fi

# Marker for smoke tooling that expects lib/ layout (no .a required for pure Ori).
printf 'ori-imguidialog pure-ori 0.1.0\n' > "$out/PURE_ORI.txt"
echo "built imguidialog → pure Ori (no native) @ $out"
ls -la "$out"
