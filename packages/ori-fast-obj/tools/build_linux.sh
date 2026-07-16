#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
cc -O2 -fPIC -I"$root/vendor" -c -o /tmp/ori_fast_obj_shim.o "$root/native/ori_fast_obj_shim.c"
ar rcs "$out/libori_fast_obj_shim.a" /tmp/ori_fast_obj_shim.o
cc -shared -O2 -fPIC -I"$root/vendor" -o "$out/libori_fast_obj_shim.so" "$root/native/ori_fast_obj_shim.c" -lm
echo "built fast_obj → $out"
ls -la "$out"
