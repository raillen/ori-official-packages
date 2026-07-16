#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
cc -c -O2 -fPIC -I"$root/vendor" -include stdint.h -o /tmp/ori_rres_shim.o "$root/native/ori_rres_shim.c"
ar rcs "$out/librres.a" /tmp/ori_rres_shim.o
echo "built rres → $out/librres.a"
