#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out_dir="$root/lib/$triple"
mkdir -p "$out_dir"
inc_raylib="$root/vendor/raylib"
inc_raygui="$root/vendor"
tmp=$(mktemp -d)
cc -c -O2 -fPIC \
  -I"$inc_raylib" -I"$inc_raygui" \
  -o "$tmp/raygui_impl.o" \
  "$root/native/raygui_impl.c"
ar rcs "$out_dir/libraygui.a" "$tmp/raygui_impl.o"
rm -rf "$tmp"
echo "built $out_dir/libraygui.a"
ls -la "$out_dir/libraygui.a"
