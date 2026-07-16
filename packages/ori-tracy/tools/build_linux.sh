#!/usr/bin/env sh
# Dual path (default):
#   libori_tracy_shim.{a,so}  — headless counters (JIT smoke / ori test)
#   libori_tracy_full.a       — FULL TracyClient (product AOT / Tracy GUI)
# ORI_TRACY_FULL=0  → headless only
# ORI_TRACY_FULL=1  → force FULL artifact (same dual; primary remains headless)
# ORI_TRACY_FULL=auto (default) → build FULL when vendor TracyClient.cpp exists
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
rm -f "$out/libori_tracy_shim.a" "$out/libori_tracy_shim.so" "$out/libori_tracy_full.a"

PUB="$root/vendor/public"
CLIENT="$PUB/TracyClient.cpp"
CXXFLAGS_BASE="-O2 -fPIC -std=c++17 -I$PUB"

want_full=0
if [ "${ORI_TRACY_FULL:-auto}" = "0" ]; then
  want_full=0
elif [ "${ORI_TRACY_FULL:-auto}" = "1" ]; then
  want_full=1
elif [ -f "$CLIENT" ]; then
  want_full=1
fi

# --- headless primary (always) ---
c++ $CXXFLAGS_BASE -c -o /tmp/ori_tracy_shim_hl.o "$root/native/ori_tracy_shim.cpp"
ar rcs "$out/libori_tracy_shim.a" /tmp/ori_tracy_shim_hl.o
c++ -shared $CXXFLAGS_BASE -o "$out/libori_tracy_shim.so" "$root/native/ori_tracy_shim.cpp" -lpthread

full_status=0
if [ "$want_full" = "1" ]; then
  if [ ! -f "$CLIENT" ]; then
    echo "ORI_TRACY_FULL requested but vendor TracyClient missing at $CLIENT" >&2
    exit 1
  fi
  FULL_FLAGS="$CXXFLAGS_BASE -DORI_TRACY_FULL -DTRACY_ENABLE -DTRACY_NO_EXIT"
  c++ $FULL_FLAGS -c -o /tmp/TracyClient.o "$CLIENT"
  c++ $FULL_FLAGS -c -o /tmp/ori_tracy_shim_full.o "$root/native/ori_tracy_shim.cpp"
  ar rcs "$out/libori_tracy_full.a" /tmp/ori_tracy_shim_full.o /tmp/TracyClient.o
  full_status=1
  if ! nm "$out/libori_tracy_full.a" 2>/dev/null | grep -q 'ori_tracy_zone_begin'; then
    echo "FULL archive missing ori_tracy_zone_begin" >&2
    exit 1
  fi
  if ! nm "$out/libori_tracy_full.a" 2>/dev/null | grep -q '___tracy_emit_zone_begin'; then
    echo "FULL archive missing Tracy client symbols" >&2
    exit 1
  fi
fi

echo "built tracy → $out (headless shim + FULL_A=$full_status)"
ls -la "$out"
