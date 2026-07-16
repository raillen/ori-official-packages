#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
CXXFLAGS="-O2 -fPIC -std=c++17 -I$root/vendor"
# Fresh archive each build so FULL ↔ headless does not leave stale objects.
rm -f "$out/libori_imnodes_shim.a" "$out/libori_imnodes_shim.so"
if [ "${ORI_IMNODES_FULL:-0}" = "1" ]; then
  imgui_root=$(CDPATH= cd -- "$root/../ori-imgui" && pwd)
  IMGUI="$imgui_root/vendor/cimgui/imgui"
  CXXFLAGS="$CXXFLAGS -I$IMGUI -DIMGUI_DEFINE_MATH_OPERATORS -DORI_IMNODES_FULL"
  c++ $CXXFLAGS -c -o /tmp/imnodes.o "$root/vendor/imnodes.cpp"
  c++ $CXXFLAGS -c -o /tmp/ori_imnodes_shim.o "$root/native/ori_imnodes_shim.cpp"
  ar rcs "$out/libori_imnodes_shim.a" /tmp/ori_imnodes_shim.o /tmp/imnodes.o
  c++ -shared $CXXFLAGS -o "$out/libori_imnodes_shim.so" /tmp/ori_imnodes_shim.o /tmp/imnodes.o
else
  c++ $CXXFLAGS -c -o /tmp/ori_imnodes_shim.o "$root/native/ori_imnodes_shim.cpp"
  ar rcs "$out/libori_imnodes_shim.a" /tmp/ori_imnodes_shim.o
  c++ -shared $CXXFLAGS -o "$out/libori_imnodes_shim.so" "$root/native/ori_imnodes_shim.cpp"
fi
echo "built imnodes → $out (FULL=${ORI_IMNODES_FULL:-0})"
ls -la "$out"
