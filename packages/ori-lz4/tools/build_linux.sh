#!/usr/bin/env sh
# Build ori_lz4_shim for Linux x86_64.
# Prefer system liblz4 (pkg-config liblz4); else compile vendored lz4.c + shim.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"

shim_c="$root/native/ori_lz4_shim.c"
vendor="$root/vendor"

use_system=0
if pkg-config --exists liblz4 2>/dev/null; then
  use_system=1
fi

if [ "$use_system" = 1 ]; then
  cflags="$(pkg-config --cflags liblz4) -DORI_LZ4_SYSTEM"
  libs=$(pkg-config --libs liblz4)
  # shellcheck disable=SC2086
  cc -O2 -fPIC $cflags -c -o /tmp/ori_lz4_shim.o "$shim_c"
  ar rcs "$out/libori_lz4_shim_objs.a" /tmp/ori_lz4_shim.o
  # shellcheck disable=SC2086
  cc -shared -O2 -fPIC $cflags -o "$out/libori_lz4_shim.so" "$shim_c" $libs
  abs=$(CDPATH= cd -- "$out" && pwd)
  # AOT: pull system -llz4 via GNU ld script
  printf '/* GNU ld script */\nINPUT ( %s/libori_lz4_shim_objs.a %s )\n' "$abs" "$libs" \
    > "$out/libori_lz4_shim.a"
  echo "built lz4 (system) → $out"
else
  cflags="-O2 -fPIC -I$vendor"
  cc $cflags -c -o /tmp/lz4.o "$vendor/lz4.c"
  cc $cflags -c -o /tmp/ori_lz4_shim.o "$shim_c"
  ar rcs "$out/libori_lz4_shim.a" /tmp/ori_lz4_shim.o /tmp/lz4.o
  cc -shared $cflags -o "$out/libori_lz4_shim.so" \
    /tmp/ori_lz4_shim.o /tmp/lz4.o
  echo "built lz4 (vendored) → $out"
fi
ls -la "$out"
