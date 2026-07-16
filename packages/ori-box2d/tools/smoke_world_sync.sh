#!/usr/bin/env sh
# Headless Box2D ↔ game.world sync smoke (F physics full path-dep).
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
pkg="$root/examples/world_sync_pkg"
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_AOT="${ORI_USE_AOT:-1}"

triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
mkdir -p "$pkg/lib/$triple"
cp -f "$root/lib/$triple/libori_box2d_shim.a" "$pkg/lib/$triple/"
cp -f "$root/lib/$triple/libbox2d.a" "$pkg/lib/$triple/"
# raylib path-dep may pull via ori_game — stage if present at game root
game_lib="$root/../ori-game/lib/$triple"
ray_lib="$root/../ori-raylib/lib/$triple"
if [ -d "$game_lib" ]; then
  for f in "$game_lib"/*.a; do
    [ -f "$f" ] && cp -f "$f" "$pkg/lib/$triple/" 2>/dev/null || true
  done
fi
if [ -d "$ray_lib" ]; then
  for f in "$ray_lib"/*.a; do
    [ -f "$f" ] && cp -f "$f" "$pkg/lib/$triple/" 2>/dev/null || true
  done
fi

export LIBRARY_PATH="$pkg/lib/$triple${LIBRARY_PATH:+:$LIBRARY_PATH}"
export LD_LIBRARY_PATH="$pkg/lib/$triple${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

"$ORI_BIN" check "$pkg/main.orl"
out=$(mktemp)
"$ORI_BIN" compile "$pkg/main.orl" --out "$out"
result=$("$out")
rm -f "$out"
echo "$result"
echo "$result" | grep -qx ok
echo "smoke_world_sync: ok"
