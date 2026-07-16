#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
V="$root/vendor"
# Memory-only zip: no stdio/time. Archive APIs enabled for create/extract one entry.
CFLAGS="-O2 -fPIC -I$V -DMINIZ_NO_STDIO -DMINIZ_NO_TIME"
objs=""
for src in miniz.c miniz_tdef.c miniz_tinfl.c miniz_zip.c; do
  cc $CFLAGS -c -o "/tmp/$src.o" "$V/$src"
  objs="$objs /tmp/$src.o"
done
cc $CFLAGS -c -o /tmp/ori_miniz_shim.o "$root/native/ori_miniz_shim.c"
# shellcheck disable=SC2086
ar rcs "$out/libori_miniz_shim.a" /tmp/ori_miniz_shim.o $objs
# shellcheck disable=SC2086
cc -shared $CFLAGS -o "$out/libori_miniz_shim.so" \
  "$root/native/ori_miniz_shim.c" \
  "$V/miniz.c" "$V/miniz_tdef.c" "$V/miniz_tinfl.c" "$V/miniz_zip.c" -lm
echo "built miniz → $out"
ls -la "$out"
