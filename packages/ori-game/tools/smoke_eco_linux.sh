#!/usr/bin/env sh
# Umbrella smoke: each ECO package smoke + cross-package integration demos.
# Continues after individual failures; exits non-zero if any package failed.
#
# Layout: proj_root = parent of ori-game (= game-engine-full/). Each package is
#   $proj_root/ori-<name>/ with ./tools/smoke_linux.sh (missing dir/script → SKIP).
#
# Env:
#   ORI_BIN                 path to ori compiler (required unless `ori` on PATH)
#   ORI_USE_SYSTEM_LINKER   default 1 (set by this script if unset)
#   ECO_SMOKE_SKIP_GAME=1   skip ori-game full smoke (ports-only)
#   ECO_SMOKE_SKIP_DEMOS=1  skip integration demos (box2d_visual, jolt_boxes_3d, …)
#
# Packages covered: raylib, game, box2d, jolt, imgui, raygui, rres, sqlite, enet,
#   freetype, harfbuzz, stb, noise, miniz, nfd, implot, imnodes, imguizmo, tracy,
#   enkiTS, cgltf, fast-obj, physfs, clay, lz4, recast,
#   imguidialog, imgui-extras, imgui-texinspect, imgui-textedit, imgui-widgets,
#   imgui-memory, implot3d, imgui-markdown
#   (imguizmo 0.3, imgui 0.5.x; missing dir/script → SKIP; + optional demos).
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

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
game_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
proj_root=$(CDPATH= cd -- "$game_root/.." && pwd)

ok_count=0
fail_count=0
skip_count=0

# ori-raylib smoke installs a headless *stub* that can overwrite a full
# libraylib.a. Restore full artifacts from ori-game when present so path-deps
# used by later packages (and ori-game) keep a real raylib.
restore_full_raylib_if_staged() {
    triple=x86_64-unknown-linux-gnu
    src="$game_root/lib/$triple"
    dst="$proj_root/ori-raylib/lib/$triple"
    if [ -f "$src/libraylib.a" ] && [ -f "$src/libori_raylib_shim.a" ] \
        && [ -d "$dst" ]; then
        src_sz=$(wc -c < "$src/libraylib.a")
        dst_sz=0
        if [ -f "$dst/libraylib.a" ]; then
            dst_sz=$(wc -c < "$dst/libraylib.a")
        fi
        # Full raylib is multi-MB; stub dummy is ~1KB.
        if [ "$src_sz" -gt 100000 ] && [ "$dst_sz" -lt 100000 ]; then
            echo "restoring full raylib into ori-raylib (stub was smaller)"
            cp -a "$src/libraylib.a" "$dst/libraylib.a"
            cp -a "$src/libori_raylib_shim.a" "$dst/libori_raylib_shim.a"
            if [ -f "$src/raylib.h" ]; then
                cp -a "$src/raylib.h" "$dst/raylib.h"
            fi
        fi
    fi
}

run_pkg_smoke() {
    name=$1
    dir=$2
    smoke=$3
    echo ""
    echo "======== ECO smoke: $name ========"
    if [ ! -d "$dir" ]; then
        echo "SKIP $name (missing $dir)"
        skip_count=$((skip_count + 1))
        return 0
    fi
    if [ ! -f "$dir/$smoke" ]; then
        echo "SKIP $name (no $smoke)"
        skip_count=$((skip_count + 1))
        return 0
    fi
    if (CDPATH= cd -- "$dir" && ORI_BIN="$ORI_BIN" sh "$smoke"); then
        echo "OK $name"
        ok_count=$((ok_count + 1))
        return 0
    fi
    echo "FAIL $name"
    fail_count=$((fail_count + 1))
    return 0
}

# Core Linux-5 stack
run_pkg_smoke "ori-raylib" "$proj_root/ori-raylib" "./tools/smoke_linux.sh"
restore_full_raylib_if_staged

if [ "${ECO_SMOKE_SKIP_GAME:-0}" = "1" ]; then
    echo ""
    echo "======== ECO smoke: ori-game ========"
    echo "SKIP ori-game (ECO_SMOKE_SKIP_GAME=1)"
    skip_count=$((skip_count + 1))
else
    run_pkg_smoke "ori-game" "$game_root" "./tools/smoke_linux.sh"
fi

run_pkg_smoke "ori-box2d" "$proj_root/ori-box2d" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-jolt" "$proj_root/ori-jolt" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imgui" "$proj_root/ori-imgui" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-raygui" "$proj_root/ori-raygui" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-rres" "$proj_root/ori-rres" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-sqlite" "$proj_root/ori-sqlite" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-enet" "$proj_root/ori-enet" "./tools/smoke_linux.sh"

# High ports (B2.18 alta) — SKIP if directory missing
run_pkg_smoke "ori-freetype" "$proj_root/ori-freetype" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-harfbuzz" "$proj_root/ori-harfbuzz" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-stb" "$proj_root/ori-stb" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-noise" "$proj_root/ori-noise" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-miniz" "$proj_root/ori-miniz" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-nfd" "$proj_root/ori-nfd" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-implot" "$proj_root/ori-implot" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imnodes" "$proj_root/ori-imnodes" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imguizmo" "$proj_root/ori-imguizmo" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-tracy" "$proj_root/ori-tracy" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-enkiTS" "$proj_root/ori-enkiTS" "./tools/smoke_linux.sh"

# Medium ports (B2.18 média M1–M6) — SKIP if directory missing
run_pkg_smoke "ori-cgltf" "$proj_root/ori-cgltf" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-fast-obj" "$proj_root/ori-fast-obj" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-physfs" "$proj_root/ori-physfs" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-clay" "$proj_root/ori-clay" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-lz4" "$proj_root/ori-lz4" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-recast" "$proj_root/ori-recast" "./tools/smoke_linux.sh"

# ImGui tools residual (plan a68f7529 A–D) — SKIP if directory missing
# Note: imguizmo 0.3 + imgui 0.5.x already covered above under high/core.
run_pkg_smoke "ori-imguidialog" "$proj_root/ori-imguidialog" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imgui-extras" "$proj_root/ori-imgui-extras" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imgui-texinspect" "$proj_root/ori-imgui-texinspect" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imgui-textedit" "$proj_root/ori-imgui-textedit" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imgui-widgets" "$proj_root/ori-imgui-widgets" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imgui-memory" "$proj_root/ori-imgui-memory" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-implot3d" "$proj_root/ori-implot3d" "./tools/smoke_linux.sh"
run_pkg_smoke "ori-imgui-markdown" "$proj_root/ori-imgui-markdown" "./tools/smoke_linux.sh"

if [ "${ECO_SMOKE_SKIP_DEMOS:-0}" = "1" ]; then
    echo ""
    echo "======== Integration demos ========"
    echo "SKIP demos (ECO_SMOKE_SKIP_DEMOS=1)"
else
    echo ""
    echo "======== Integration demos ========"
    run_pkg_smoke "box2d_visual" "$proj_root/ori-box2d/demos/box2d_visual" "./tools/smoke.sh"
    run_pkg_smoke "jolt_boxes_3d" "$proj_root/ori-jolt/demos/jolt_boxes_3d" "./tools/smoke.sh"
    run_pkg_smoke "raygui_game" "$proj_root/ori-raygui/demos/raygui_game" "./tools/smoke.sh"
    run_pkg_smoke "score_game" "$proj_root/ori-sqlite/demos/score_game" "./tools/smoke.sh"
    run_pkg_smoke "imgui_game" "$proj_root/ori-imgui/demos/imgui_game" "./tools/smoke.sh"
    run_pkg_smoke "rres_assets" "$game_root/demos/rres_assets" "./tools/smoke.sh"
fi

echo ""
echo "smoke_eco_linux: ok=$ok_count fail=$fail_count skip=$skip_count"
if [ "$fail_count" -gt 0 ]; then
    echo "smoke_eco_linux: FAILED"
    exit 1
fi
echo "smoke_eco_linux: all green"
exit 0
