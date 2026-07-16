#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
cc -O2 -fPIC -I"$root/vendor" -c -o /tmp/ori_stb_shim.o "$root/native/ori_stb_shim.c"
ar rcs "$out/libori_stb_shim.a" /tmp/ori_stb_shim.o
cc -shared -O2 -fPIC -I"$root/vendor" -o "$out/libori_stb_shim.so" "$root/native/ori_stb_shim.c" -lm
echo "built stb → $out"
ls -la "$out"
