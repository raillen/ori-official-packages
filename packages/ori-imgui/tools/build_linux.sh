#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
export PATH="${CMAKE_BIN_DIR:-/tmp/cmake-3.30.5-linux-x86_64/bin}:$PATH"
OUT="$root/lib/x86_64-unknown-linux-gnu"
IMGUI="$root/vendor/cimgui/imgui"
GLFW_INC="$root/vendor/glfw/include"
mkdir -p "$OUT"

if [ ! -d "$root/vendor/cimgui/imgui" ]; then
  echo "missing vendor/cimgui (git clone --recursive)" >&2
  exit 1
fi
if [ ! -f "$root/vendor/glfw/build/src/libglfw3.a" ]; then
  cmake -S "$root/vendor/glfw" -B "$root/vendor/glfw/build" \
    -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release
  cmake --build "$root/vendor/glfw/build" -j"$(nproc 2>/dev/null || echo 2)"
fi

# imgui_demo.cpp provides ShowStyleEditor / ShowDemoWindow (used by Tier2 host API).
for src in imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp; do
  c++ -O2 -fPIC -std=c++17 -I"$IMGUI" -I"$IMGUI/backends" \
    -c -o "/tmp/${src%.cpp}.o" "$IMGUI/$src"
done
c++ -O2 -fPIC -std=c++17 -I"$IMGUI" -I"$IMGUI/backends" -I"$GLFW_INC" \
  -c -o /tmp/imgui_impl_glfw.o "$IMGUI/backends/imgui_impl_glfw.cpp"
c++ -O2 -fPIC -std=c++17 -I"$IMGUI" -I"$IMGUI/backends" -I"$GLFW_INC" \
  -c -o /tmp/imgui_impl_opengl3.o "$IMGUI/backends/imgui_impl_opengl3.cpp"
# Raylib embed (P2-A): opt-in only. Default package is GLFW-only so demos link without raylib.
# Set ORI_IMGUI_WITH_RAYLIB=1 (and optional ORI_RAYLIB_INC) for imgui_game demos.
HOST_FLAGS="-I$IMGUI -I$IMGUI/backends -I$GLFW_INC"
if [ "${ORI_IMGUI_WITH_RAYLIB:-0}" = "1" ]; then
  RAYLIB_INC="${ORI_RAYLIB_INC:-}"
  if [ -z "$RAYLIB_INC" ]; then
    for cand in \
      "$root/../ori-raygui/vendor/raylib" \
      "$root/../ori-game/lib/x86_64-unknown-linux-gnu" \
      /usr/include
    do
      if [ -f "$cand/raylib.h" ]; then
        RAYLIB_INC="$cand"
        break
      fi
    done
  fi
  if [ -n "$RAYLIB_INC" ]; then
    HOST_FLAGS="$HOST_FLAGS -I$RAYLIB_INC"
    echo "raylib embed: ON (headers at $RAYLIB_INC)"
  else
    HOST_FLAGS="$HOST_FLAGS -DORI_IMGUI_NO_RAYLIB"
    echo "raylib embed requested but headers missing — NO_RAYLIB"
  fi
else
  HOST_FLAGS="$HOST_FLAGS -DORI_IMGUI_NO_RAYLIB"
  echo "raylib embed: OFF (default GLFW host; set ORI_IMGUI_WITH_RAYLIB=1 for game embed)"
fi

# shellcheck disable=SC2086
c++ -O2 -fPIC -std=c++17 $HOST_FLAGS \
  -c -o /tmp/ori_imgui_host.o "$root/native/ori_imgui_host.cpp"

ar rcs "$OUT/libori_imgui.a" \
  /tmp/ori_imgui_host.o /tmp/imgui_impl_glfw.o /tmp/imgui_impl_opengl3.o \
  /tmp/imgui.o /tmp/imgui_draw.o /tmp/imgui_tables.o /tmp/imgui_widgets.o /tmp/imgui_demo.o
cp -f "$root/vendor/glfw/build/src/libglfw3.a" "$OUT/libglfw3.a"
cat > "$OUT/libsysdeps.a" << 'SCRIPT'
/* GNU ld script — system deps for ori-imgui */
INPUT ( -lGL -lstdc++ -lX11 -lpthread -ldl -lm -lXrandr -lXi -lXcursor -lXinerama )
SCRIPT
echo "built imgui → $OUT/libori_imgui.a"
