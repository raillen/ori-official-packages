#!/usr/bin/env sh
# Build ori_freetype_shim against system FreeType (Linux).
# Stages static + shared deps so AOT and JIT package native_libs resolve.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"

if ! pkg-config --exists freetype2; then
  echo "freetype2 not found (pkg-config). Install libfreetype6-dev." >&2
  exit 1
fi

FT_CFLAGS=$(pkg-config --cflags freetype2)
FT_LIBS=$(pkg-config --libs freetype2)
CFLAGS="-O2 -fPIC $FT_CFLAGS"

echo "building → $out"
cc -c $CFLAGS -o /tmp/ori_freetype_shim.o "$root/native/ori_freetype_shim.c"
ar rcs "$out/libori_freetype_shim.a" /tmp/ori_freetype_shim.o

cc -shared $CFLAGS \
  -o "$out/libori_freetype_shim.so" \
  "$root/native/ori_freetype_shim.c" \
  $FT_LIBS -Wl,-rpath,'$ORIGIN'

stage_lib() {
  name="$1"
  staged=0
  for candidate in \
    "/usr/lib/x86_64-linux-gnu/lib${name}.a" \
    "/usr/lib/lib${name}.a"
  do
    if [ -f "$candidate" ]; then
      cp -f "$candidate" "$out/lib${name}.a"
      staged=1
      break
    fi
  done
  # Shared: prefer unversioned soname path, then common versioned names.
  for candidate in \
    "/usr/lib/x86_64-linux-gnu/lib${name}.so" \
    "/lib/x86_64-linux-gnu/lib${name}.so" \
    "/usr/lib/x86_64-linux-gnu/lib${name}.so."* \
    "/lib/x86_64-linux-gnu/lib${name}.so."*
  do
    if [ -e "$candidate" ]; then
      # Copy real file content for a stable libNAME.so load path (JIT).
      cp -fL "$candidate" "$out/lib${name}.so" 2>/dev/null || true
      if [ -f "$out/lib${name}.so" ]; then
        staged=1
        break
      fi
    fi
  done
  if [ "$staged" -eq 0 ]; then
    echo "warn: lib${name} not found (AOT/JIT may fail)" >&2
  else
    echo "staged $name"
  fi
}

stage_lib freetype
stage_lib png16
stage_lib z
stage_lib bz2
stage_lib brotlidec
stage_lib brotlicommon

echo "built freetype shim → $out"
ls -la "$out"
