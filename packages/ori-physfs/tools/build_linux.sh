#!/usr/bin/env sh
# Build ori_physfs_shim for Linux x86_64.
# Prefer system libphysfs (pkg-config physfs); else compile vendored PhysFS static.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"

shim_c="$root/native/ori_physfs_shim.c"
vendor="$root/vendor/physfs"

use_system=0
if pkg-config --exists physfs 2>/dev/null; then
  use_system=1
fi

if [ "$use_system" = 1 ]; then
  cflags="$(pkg-config --cflags physfs) -DORI_PHYSFS_SYSTEM"
  libs=$(pkg-config --libs physfs)
  # shellcheck disable=SC2086
  cc -O2 -fPIC $cflags -c -o /tmp/ori_physfs_shim.o "$shim_c"
  ar rcs "$out/libori_physfs_shim_objs.a" /tmp/ori_physfs_shim.o
  # shellcheck disable=SC2086
  cc -shared -O2 -fPIC $cflags -o "$out/libori_physfs_shim.so" "$shim_c" $libs
  abs=$(CDPATH= cd -- "$out" && pwd)
  # AOT: pull system -lphysfs via GNU ld script
  printf '/* GNU ld script */\nINPUT ( %s/libori_physfs_shim_objs.a %s )\n' "$abs" "$libs" \
    > "$out/libori_physfs_shim.a"
  echo "built physfs (system) → $out"
else
  # Vendored: compile all PhysFS .c (platform bits #ifdef out) + shim
  cflags="-O2 -fPIC -I$vendor"
  objs=""
  for f in "$vendor"/*.c; do
    base=$(basename "$f" .c)
    # shellcheck disable=SC2086
    cc $cflags -c -o "/tmp/pf_$base.o" "$f"
    objs="$objs /tmp/pf_$base.o"
  done
  # shellcheck disable=SC2086
  cc $cflags -c -o /tmp/ori_physfs_shim.o "$shim_c"
  # shellcheck disable=SC2086
  ar rcs "$out/libori_physfs_shim.a" /tmp/ori_physfs_shim.o $objs
  # shellcheck disable=SC2086
  cc -shared $cflags -o "$out/libori_physfs_shim.so" \
    /tmp/ori_physfs_shim.o $objs -lpthread
  echo "built physfs (vendored) → $out"
fi
ls -la "$out"
