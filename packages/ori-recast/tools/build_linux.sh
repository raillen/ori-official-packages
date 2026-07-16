#!/usr/bin/env sh
# Build libori_recast_shim (.a ld-script + .so) from Recast + Detour + C++ shim.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
R="$root/vendor/recastnavigation"
mkdir -p "$out"

if [ ! -d "$R/Recast/Source" ] || [ ! -d "$R/Detour/Source" ]; then
  echo "vendor/recastnavigation missing (clone recastnavigation)" >&2
  exit 1
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

CXXFLAGS="-O2 -fPIC -std=c++17 -fno-exceptions -fno-rtti -DNDEBUG"
INC="-I$R/Recast/Include -I$R/Detour/Include"

objs=""
for f in "$R/Recast/Source"/*.cpp; do
  base=$(basename "$f" .cpp)
  o="$tmp/${base}.o"
  # shellcheck disable=SC2086
  g++ $CXXFLAGS $INC -c -o "$o" "$f"
  objs="$objs $o"
done
for f in "$R/Detour/Source"/*.cpp; do
  base=$(basename "$f" .cpp)
  o="$tmp/${base}.o"
  # shellcheck disable=SC2086
  g++ $CXXFLAGS $INC -c -o "$o" "$f"
  objs="$objs $o"
done

# shellcheck disable=SC2086
g++ $CXXFLAGS $INC -c -o "$tmp/ori_recast_shim.o" "$root/native/ori_recast_shim.cpp"
objs="$objs $tmp/ori_recast_shim.o"

# Real objects archive
# shellcheck disable=SC2086
ar rcs "$out/libori_recast_objs.a" $objs

# Shared for JIT
# shellcheck disable=SC2086
g++ -shared -O2 -fPIC -o "$out/libori_recast_shim.so" $objs -lstdc++ -lm

# AOT: GNU ld script as package native_lib name → objs + libstdc++
abs=$(CDPATH= cd -- "$out" && pwd)
printf '/* GNU ld script */\nINPUT ( %s/libori_recast_objs.a -lstdc++ -lm )\n' "$abs" \
  > "$out/libori_recast_shim.a"

echo "built recast → $out"
nm -D "$out/libori_recast_shim.so" | grep ' T ori_recast_build_plane' || {
  echo "missing ori_recast_build_plane" >&2
  exit 1
}
nm -D "$out/libori_recast_shim.so" | grep ' T ori_recast_build_mesh' || {
  echo "missing ori_recast_build_mesh" >&2
  exit 1
}
nm -D "$out/libori_recast_shim.so" | grep ' T ori_recast_find_nearest' || {
  echo "missing ori_recast_find_nearest" >&2
  exit 1
}
nm -D "$out/libori_recast_shim.so" | grep ' T ori_recast_raycast' || {
  echo "missing ori_recast_raycast" >&2
  exit 1
}
ls -la "$out"
