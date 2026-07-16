#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
export PATH="${CMAKE_BIN_DIR:-/tmp/cmake-3.30.5-linux-x86_64/bin}:$PATH"
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
if [ ! -f "$root/vendor/box2d/build/src/libbox2d.a" ]; then
  cmake -S "$root/vendor/box2d" -B "$root/vendor/box2d/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF -DBOX2D_BENCHMARKS=OFF \
    -DCMAKE_C_FLAGS="-Wno-error -Wno-maybe-uninitialized"
  cmake --build "$root/vendor/box2d/build" -j"$(nproc 2>/dev/null || echo 2)"
fi
cp -f "$root/vendor/box2d/build/src/libbox2d.a" "$out/libbox2d.a"
cc -c -O2 -fPIC -I"$root/vendor/box2d/include" -o /tmp/ori_box2d_shim.o "$root/native/ori_box2d_shim.c"
ar rcs "$out/libori_box2d_shim.a" /tmp/ori_box2d_shim.o
echo "built box2d + shim → $out"
