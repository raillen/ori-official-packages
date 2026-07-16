#!/usr/bin/env sh
# P3 — Umbrella macOS ECO smoke.
# Prefer per-package tools/smoke_macos.sh; fall back to smoke_linux.sh when
# the package smoke is triple-agnostic (check+compile). Missing → SKIP.
#
#   export ORI_BIN=ori ORI_USE_SYSTEM_LINKER=1
#   ./tools/smoke_eco_macos.sh
#
# Env:
#   ORI_HOST_TRIPLE   force triple (default: detect arm64/x86_64-apple-darwin)
#   ECO_SMOKE_SKIP_GAME=1
#   ECO_SMOKE_CORE_ONLY=1   only PHASE-OS core 8 + game
set -u

ORI_BIN="${ORI_BIN:-}"
if [ -z "$ORI_BIN" ]; then
    if command -v ori >/dev/null 2>&1; then
        ORI_BIN=$(command -v ori)
    else
        echo "set ORI_BIN to ori compiler" >&2
        exit 2
    fi
fi
export ORI_BIN
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_AOT="${ORI_USE_AOT:-1}"

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
game_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
proj_root=$(CDPATH= cd -- "$game_root/.." && pwd)

if [ -z "${ORI_HOST_TRIPLE:-}" ]; then
    arch=$(uname -m 2>/dev/null || echo x86_64)
    case "$arch" in
        arm64|aarch64) ORI_HOST_TRIPLE="aarch64-apple-darwin" ;;
        *) ORI_HOST_TRIPLE="x86_64-apple-darwin" ;;
    esac
fi
export ORI_HOST_TRIPLE
echo "smoke_eco_macos · triple=$ORI_HOST_TRIPLE · proj=$proj_root"

ok_count=0
fail_count=0
skip_count=0

run_pkg_smoke() {
    name=$1
    dir=$2
    echo ""
    echo "======== ECO macOS smoke: $name ========"
    if [ ! -d "$dir" ]; then
        echo "SKIP $name (missing $dir)"
        skip_count=$((skip_count + 1))
        return 0
    fi
    smoke=""
    if [ -f "$dir/tools/smoke_macos.sh" ]; then
        smoke="./tools/smoke_macos.sh"
    elif [ -f "$dir/tools/smoke_linux.sh" ]; then
        # Many packages only check+compile; reuse on Darwin when triple is set.
        smoke="./tools/smoke_linux.sh"
        echo "NOTE $name: using smoke_linux.sh on mac (no smoke_macos.sh yet)"
    else
        echo "SKIP $name (no smoke_macos.sh / smoke_linux.sh)"
        skip_count=$((skip_count + 1))
        return 0
    fi
    if (CDPATH= cd -- "$dir" && ORI_BIN="$ORI_BIN" ORI_HOST_TRIPLE="$ORI_HOST_TRIPLE" sh "$smoke"); then
        echo "OK $name"
        ok_count=$((ok_count + 1))
        return 0
    fi
    echo "FAIL $name"
    fail_count=$((fail_count + 1))
    return 0
}

# Core
if [ "${ECO_SMOKE_SKIP_GAME:-0}" = "1" ]; then
    echo "SKIP ori-game (ECO_SMOKE_SKIP_GAME=1)"
    skip_count=$((skip_count + 1))
else
    run_pkg_smoke "ori-game" "$game_root"
fi

run_pkg_smoke "ori-box2d" "$proj_root/ori-box2d"
run_pkg_smoke "ori-jolt" "$proj_root/ori-jolt"
run_pkg_smoke "ori-sqlite" "$proj_root/ori-sqlite"
run_pkg_smoke "ori-rres" "$proj_root/ori-rres"
run_pkg_smoke "ori-imgui" "$proj_root/ori-imgui"
run_pkg_smoke "ori-raygui" "$proj_root/ori-raygui"
run_pkg_smoke "ori-enet" "$proj_root/ori-enet"

if [ "${ECO_SMOKE_CORE_ONLY:-0}" != "1" ]; then
    run_pkg_smoke "ori-nfd" "$proj_root/ori-nfd"
    run_pkg_smoke "ori-imguizmo" "$proj_root/ori-imguizmo"
    run_pkg_smoke "ori-imnodes" "$proj_root/ori-imnodes"
    run_pkg_smoke "ori-implot" "$proj_root/ori-implot"
    run_pkg_smoke "ori-noise" "$proj_root/ori-noise"
    run_pkg_smoke "ori-cgltf" "$proj_root/ori-cgltf"
    run_pkg_smoke "ori-fast-obj" "$proj_root/ori-fast-obj"
    run_pkg_smoke "studio_shell" "$proj_root/ori-imgui/demos/studio_shell"
fi

echo ""
echo "smoke_eco_macos: ok=$ok_count fail=$fail_count skip=$skip_count"
if [ "$fail_count" -gt 0 ]; then
    echo "smoke_eco_macos: FAILED"
    exit 1
fi
echo "smoke_eco_macos: all green"
exit 0
