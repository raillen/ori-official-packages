#!/usr/bin/env sh
# Linux smoke (trilha G): check lib + canonic demos, unit tests, compile+run.
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)

ORI_BIN="${ORI_BIN:-}"
if [ -z "$ORI_BIN" ]; then
    if command -v ori >/dev/null 2>&1; then
        ORI_BIN=$(command -v ori)
    else
        echo "ori compiler not found; set ORI_BIN" >&2
        exit 2
    fi
fi

export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"

# Ensure scalar shim + raylib (stub or real) exist
if [ ! -f "$repo_root/lib/x86_64-unknown-linux-gnu/libori_raylib_shim.a" ] \
    || [ ! -f "$repo_root/lib/x86_64-unknown-linux-gnu/libraylib.a" ]; then
    "$script_dir/setup_raylib_linux.sh" --stub
fi

check_one() {
    f=$1
    if "$ORI_BIN" check "$f" >/dev/null 2>&1; then
        return 0
    fi
    echo "FAIL check: $f"
    "$ORI_BIN" check "$f" 2>&1 | head -12
    return 1
}

echo "== ori check (lib + canonic demos + tests) =="
fail=0
ok=0
for f in \
    "$repo_root"/raylib.orl \
    "$repo_root"/color.orl \
    "$repo_root"/shape.orl \
    "$repo_root"/collision.orl \
    "$repo_root"/game/*.orl \
    "$repo_root"/game/mechanics/*.orl \
    "$repo_root"/tests/*.orl \
    "$repo_root"/examples/app_smoke.orl \
    "$repo_root"/examples/hello_game.orl \
    "$repo_root"/examples/simple_game.orl \
    "$repo_root"/examples/hello_3d.orl \
    "$repo_root"/examples/pick_3d.orl \
    "$repo_root"/examples/shader_light_demo.orl \
    "$repo_root"/examples/skeletal_anim_demo.orl \
    "$repo_root"/examples/light_bank_demo.orl \
    "$repo_root"/examples/anim_player_demo.orl \
    "$repo_root"/examples/platformer.orl \
    "$repo_root"/examples/tilemap_demo.orl \
    "$repo_root"/examples/particles_demo.orl \
    "$repo_root"/examples/physics_demo.orl \
    "$repo_root"/examples/inventory_demo.orl \
    "$repo_root"/examples/dialogue_demo.orl \
    "$repo_root"/examples/state_demo.orl \
    "$repo_root"/examples/audio_demo.orl \
    "$repo_root"/examples/ai_demo.orl \
    "$repo_root"/examples/space_shooter.orl \
    "$repo_root"/examples/scene_menu.orl \
    "$repo_root"/examples/gamepad_demo.orl \
    "$repo_root"/examples/render_texture_demo.orl \
    "$repo_root"/examples/tiled_demo.orl \
    "$repo_root"/game/tiled.orl \
    "$repo_root"/game/aseprite.orl \
    "$repo_root"/examples/aseprite_demo.orl \
    "$repo_root"/examples/audio_deepen_demo.orl \
    "$repo_root"/examples/shapes3d_demo.orl \
    "$repo_root"/game/rres_assets.orl \
    "$repo_root"/game/ldtk.orl \
    "$repo_root"/game/spine.orl \
    "$repo_root"/game/json_fields.orl \
    "$repo_root"/game/ogame_entity.orl \
    "$repo_root"/game/ogame_scene.orl \
    "$repo_root"/game/ogame_spawn.orl \
    "$repo_root"/game/ogame.orl \
    "$repo_root"/examples/ogame_scene_smoke.orl \
    "$repo_root"/examples/ldtk_demo.orl \
    "$repo_root"/examples/spine_demo.orl
do
    [ -f "$f" ] || continue
    if check_one "$f"; then
        ok=$((ok + 1))
    else
        fail=$((fail + 1))
    fi
done
echo "check: $ok ok / $fail fail"
if [ "$fail" -ne 0 ]; then
    exit 1
fi

echo "== ori test =="
for t in "$repo_root"/tests/*.orl; do
    echo "-- $(basename "$t") --"
    "$ORI_BIN" test "$t"
done

echo "== compile + run demos =="
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# Headless logic demo — must exit 0
src="$repo_root/examples/simple_game.orl"
out="$tmp/simple_game"
echo "-- compile simple_game --"
"$ORI_BIN" compile "$src" --out "$out"
echo "-- run simple_game --"
"$out"

# Studio content R3 — headless load sample_project + spawn
src="$repo_root/examples/ogame_scene_smoke.orl"
out="$tmp/ogame_scene_smoke"
echo "-- compile ogame_scene_smoke --"
"$ORI_BIN" compile "$src" --out "$out"
echo "-- run ogame_scene_smoke --"
"$out" | tee /tmp/ori_game_ogame_scene_smoke.log
grep -q "ogame_scene_smoke: ok" /tmp/ori_game_ogame_scene_smoke.log

# Windowed demos: success if process starts and runs a few seconds (timeout 124 OK)

run_demo() {
    demo=$1
    src="$repo_root/examples/${demo}.orl"
    [ -f "$src" ] || return 0
    out="$tmp/$demo"
    echo "-- compile $demo --"
    "$ORI_BIN" compile "$src" --out "$out"
    echo "-- run $demo (3s timeout) --"
    set +e
    timeout 3 "$out" >/tmp/ori_game_${demo}.log 2>&1
    code=$?
    set -e
    # 0 = clean exit (stub), 124 = still running after timeout (real raylib OK)
    if [ "$code" -eq 0 ] || [ "$code" -eq 124 ]; then
        echo "OK $demo (exit $code)"
    else
        echo "run failed: $demo exit $code" >&2
        tail -30 /tmp/ori_game_${demo}.log >&2 || true
        exit 1
    fi
}

run_demo app_smoke
run_demo hello_game
run_demo scene_menu
run_demo hello_3d
run_demo pick_3d
run_demo shader_light_demo
run_demo skeletal_anim_demo
run_demo light_bank_demo
run_demo anim_player_demo
run_demo platformer
run_demo tilemap_demo
run_demo particles_demo
run_demo physics_demo
run_demo inventory_demo
run_demo dialogue_demo
run_demo state_demo
run_demo audio_demo
run_demo ai_demo
run_demo space_shooter
run_demo gamepad_demo
run_demo render_texture_demo
run_demo tiled_demo
run_demo aseprite_demo
run_demo audio_deepen_demo
run_demo shapes3d_demo

echo "smoke_linux: all green"
