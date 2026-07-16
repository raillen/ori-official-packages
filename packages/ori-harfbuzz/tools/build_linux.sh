#!/usr/bin/env sh
# Build ori_harfbuzz_shim against system HarfBuzz + FreeType.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"

if ! pkg-config --exists harfbuzz; then
  echo "harfbuzz not found (pkg-config). Install libharfbuzz-dev." >&2
  exit 1
fi
if ! pkg-config --exists freetype2; then
  echo "freetype2 not found. Install libfreetype6-dev." >&2
  exit 1
fi

HB_CFLAGS=$(pkg-config --cflags harfbuzz freetype2)
HB_LIBS=$(pkg-config --libs harfbuzz freetype2)
CFLAGS="-O2 -fPIC $HB_CFLAGS"

echo "building → $out"
cc -c $CFLAGS -o /tmp/ori_harfbuzz_shim.o "$root/native/ori_harfbuzz_shim.c"
ar rcs "$out/libori_harfbuzz_shim.a" /tmp/ori_harfbuzz_shim.o

cc -shared $CFLAGS \
  -o "$out/libori_harfbuzz_shim.so" \
  "$root/native/ori_harfbuzz_shim.c" \
  $HB_LIBS -Wl,-rpath,'$ORIGIN'

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
  for candidate in \
    "/usr/lib/x86_64-linux-gnu/lib${name}.so" \
    "/lib/x86_64-linux-gnu/lib${name}.so" \
    "/usr/lib/x86_64-linux-gnu/lib${name}.so."* \
    "/lib/x86_64-linux-gnu/lib${name}.so."*
  do
    if [ -e "$candidate" ]; then
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

stage_lib harfbuzz
stage_lib graphite2
stage_lib glib-2.0
stage_lib pcre2-8

# AOT: if static .a missing, write GNU ld script that inputs the shared lib.
ensure_ld_script() {
  name="$1"
  if [ ! -f "$out/lib${name}.a" ] && [ -f "$out/lib${name}.so" ]; then
    abs=$(CDPATH= cd -- "$out" && pwd)
    printf '/* GNU ld script — shared-only distro fallback */\nINPUT ( %s/lib%s.so )\n' "$abs" "$name" > "$out/lib${name}.a"
    echo "ld-script lib${name}.a → lib${name}.so"
  fi
}
ensure_ld_script harfbuzz
ensure_ld_script graphite2

# Also list system-deps in package for AOT (optional extra staged names).
if [ -f "$out/libharfbuzz.a" ]; then
  :
fi

echo "built harfbuzz shim → $out"
ls -la "$out"
