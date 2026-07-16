#!/usr/bin/env sh
# Dual path (default when sibling ori-imgui is present):
#   libori_imguizmo_shim.{a,so}  — headless TRS + curve/gradient/zoom (smoke / ori test)
#   libori_imguizmo_full.a       — FULL ImGuizmo + ImCurveEdit + ImGradient (+ ZoomSlider header)
# ORI_IMGUIZMO_FULL=0  → headless only
# ORI_IMGUIZMO_FULL=1  → force FULL artifact (primary remains headless for smoke)
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
rm -f "$out/libori_imguizmo_shim.a" "$out/libori_imguizmo_shim.so" "$out/libori_imguizmo_full.a"

imgui_hdr="$root/../ori-imgui/vendor/cimgui/imgui/imgui.h"
want_full=0
if [ "${ORI_IMGUIZMO_FULL:-auto}" = "0" ]; then
  want_full=0
elif [ "${ORI_IMGUIZMO_FULL:-auto}" = "1" ]; then
  want_full=1
elif [ -f "$imgui_hdr" ]; then
  want_full=1
fi

CXXFLAGS_BASE="-O2 -fPIC -std=c++17 -I$root/vendor -lm"
NATIVE_SRCS="
  $root/native/ori_imguizmo_shim.cpp
  $root/native/ori_imguizmo_curve.cpp
  $root/native/ori_imguizmo_gradient.cpp
  $root/native/ori_imguizmo_zoom.cpp
"

# --- headless primary (always) ---
hl_objs=""
i=0
for src in $NATIVE_SRCS; do
  obj="/tmp/ori_imguizmo_hl_${i}.o"
  c++ $CXXFLAGS_BASE -c -o "$obj" "$src"
  hl_objs="$hl_objs $obj"
  i=$((i + 1))
done
# shellcheck disable=SC2086
ar rcs "$out/libori_imguizmo_shim.a" $hl_objs
# Shared object for JIT/smoke
so_cmd="c++ -shared $CXXFLAGS_BASE -o $out/libori_imguizmo_shim.so"
for src in $NATIVE_SRCS; do
  so_cmd="$so_cmd $src"
done
so_cmd="$so_cmd -lm"
# shellcheck disable=SC2086
eval $so_cmd

full_status=0
if [ "$want_full" = "1" ]; then
  if [ ! -f "$imgui_hdr" ]; then
    echo "ORI_IMGUIZMO_FULL requested but sibling ori-imgui headers missing at $imgui_hdr" >&2
    exit 1
  fi
  imgui_root=$(CDPATH= cd -- "$root/../ori-imgui" && pwd)
  IMGUI="$imgui_root/vendor/cimgui/imgui"
  FULL_FLAGS="-O2 -fPIC -std=c++17 -I$root/vendor -I$IMGUI -DIMGUI_DEFINE_MATH_OPERATORS -DORI_IMGUIZMO_FULL"
  full_objs=""
  i=0
  for src in $NATIVE_SRCS; do
    obj="/tmp/ori_imguizmo_full_${i}.o"
    c++ $FULL_FLAGS -c -o "$obj" "$src"
    full_objs="$full_objs $obj"
    i=$((i + 1))
  done
  c++ $FULL_FLAGS -c -o /tmp/ImGuizmo.o "$root/vendor/ImGuizmo.cpp"
  c++ $FULL_FLAGS -c -o /tmp/ImCurveEdit.o "$root/vendor/ImCurveEdit.cpp"
  c++ $FULL_FLAGS -c -o /tmp/ImGradient.o "$root/vendor/ImGradient.cpp"
  # shellcheck disable=SC2086
  ar rcs "$out/libori_imguizmo_full.a" $full_objs /tmp/ImGuizmo.o /tmp/ImCurveEdit.o /tmp/ImGradient.o
  full_status=1
  for sym in ori_gizmo_manipulate_rotate ori_gizmo_manipulate_scale \
             ori_curve_sample_m ori_gradient_add_stop_m ori_zoom_span_m; do
    if ! nm "$out/libori_imguizmo_full.a" 2>/dev/null | grep -q "$sym"; then
      echo "FULL archive missing $sym" >&2
      exit 1
    fi
  done
  if ! nm "$out/libori_imguizmo_full.a" 2>/dev/null | grep -q 'ImGuizmo'; then
    echo "FULL archive missing ImGuizmo symbols" >&2
    exit 1
  fi
  if ! nm "$out/libori_imguizmo_full.a" 2>/dev/null | grep -q 'ImCurveEdit'; then
    echo "FULL archive missing ImCurveEdit symbols" >&2
    exit 1
  fi
  if ! nm "$out/libori_imguizmo_full.a" 2>/dev/null | grep -q 'ImGradient'; then
    echo "FULL archive missing ImGradient symbols" >&2
    exit 1
  fi
fi

echo "built imguizmo → $out (headless shim + FULL_A=$full_status)"
ls -la "$out"
