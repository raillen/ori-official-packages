#!/usr/bin/env sh
# Dual path (default when sibling ori-imgui is present):
#   libori_implot3d_shim.{a,so}  — headless (JIT smoke / ori test AOT)
#   libori_implot3d_full.a       — FULL ImPlot3D objects (product AOT; link with ori-imgui)
# ORI_IMPLOT3D_FULL=0  → headless only
# ORI_IMPLOT3D_FULL=1  → also force FULL artifact (same dual; primary remains headless for smoke)
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
rm -f "$out/libori_implot3d_shim.a" "$out/libori_implot3d_shim.so" "$out/libori_implot3d_full.a"

imgui_hdr="$root/../ori-imgui/vendor/cimgui/imgui/imgui.h"
want_full=0
if [ "${ORI_IMPLOT3D_FULL:-auto}" = "0" ]; then
  want_full=0
elif [ "${ORI_IMPLOT3D_FULL:-auto}" = "1" ]; then
  want_full=1
elif [ -f "$imgui_hdr" ]; then
  want_full=1
fi

CXXFLAGS_BASE="-O2 -fPIC -std=c++17 -I$root/vendor"

# --- headless primary (always) ---
c++ $CXXFLAGS_BASE -c -o /tmp/ori_implot3d_shim_hl.o "$root/native/ori_implot3d_shim.cpp"
ar rcs "$out/libori_implot3d_shim.a" /tmp/ori_implot3d_shim_hl.o
c++ -shared $CXXFLAGS_BASE -o "$out/libori_implot3d_shim.so" "$root/native/ori_implot3d_shim.cpp"

full_status=0
if [ "$want_full" = "1" ]; then
  if [ ! -f "$imgui_hdr" ]; then
    echo "ORI_IMPLOT3D_FULL requested but sibling ori-imgui headers missing at $imgui_hdr" >&2
    exit 1
  fi
  imgui_root=$(CDPATH= cd -- "$root/../ori-imgui" && pwd)
  IMGUI="$imgui_root/vendor/cimgui/imgui"
  FULL_FLAGS="$CXXFLAGS_BASE -I$IMGUI -DIMGUI_DEFINE_MATH_OPERATORS -DORI_IMPLOT3D_FULL"
  c++ $FULL_FLAGS -c -o /tmp/implot3d.o "$root/vendor/implot3d.cpp"
  c++ $FULL_FLAGS -c -o /tmp/implot3d_items.o "$root/vendor/implot3d_items.cpp"
  c++ $FULL_FLAGS -c -o /tmp/ori_implot3d_shim_full.o "$root/native/ori_implot3d_shim.cpp"
  ar rcs "$out/libori_implot3d_full.a" /tmp/ori_implot3d_shim_full.o /tmp/implot3d.o /tmp/implot3d_items.o
  full_status=1
  # Sanity: FULL archive must export scatter + ImPlot3D symbols
  if ! nm "$out/libori_implot3d_full.a" 2>/dev/null | grep -q 'ori_implot3d_plot_scatter'; then
    echo "FULL archive missing ori_implot3d_plot_scatter" >&2
    exit 1
  fi
  if ! nm "$out/libori_implot3d_full.a" 2>/dev/null | grep -q 'ImPlot3D'; then
    echo "FULL archive missing ImPlot3D symbols" >&2
    exit 1
  fi
fi

echo "built implot3d → $out (headless shim + FULL_A=$full_status)"
ls -la "$out"
