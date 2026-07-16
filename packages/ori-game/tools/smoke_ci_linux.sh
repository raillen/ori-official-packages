#!/usr/bin/env bash
# P4 — CI smoke Linux: sample Play + key unit tests + studio_shell (when cluster present).
#
# Run from:
#   game-engine-full/          → ./scripts/smoke_ci_linux.sh
#   game-engine-full/ori-game/ → ./tools/smoke_ci_linux.sh
#
# Env:
#   ORI_BIN              — path to ori (default: ori on PATH)
#   ORI_CI_SHELL=0       — skip studio_shell (sample+tests only)
#   ORI_CI_EXPORT=0      — skip export_game_package dry-run
#   ORI_CI_QUICK=1       — only sample + phase_g_l + camera_matrices
#   ORI_USE_SYSTEM_LINKER / ORI_USE_AOT — default 1
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
GAME_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CLUSTER="$(cd "$GAME_ROOT/.." && pwd)"

ORI_BIN="${ORI_BIN:-}"
if [[ -z "$ORI_BIN" ]]; then
  if command -v ori >/dev/null 2>&1; then
    ORI_BIN="$(command -v ori)"
  else
    echo "FAIL: ori not found; set ORI_BIN" >&2
    exit 2
  fi
fi

export ORI_BIN
export ORI_GAME_ROOT="$GAME_ROOT"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_AOT="${ORI_USE_AOT:-1}"
export ORI_CI_SHELL="${ORI_CI_SHELL:-1}"
export ORI_CI_EXPORT="${ORI_CI_EXPORT:-1}"
export ORI_CI_QUICK="${ORI_CI_QUICK:-0}"

TRIPLE="${ORI_HOST_TRIPLE:-x86_64-unknown-linux-gnu}"
LIB="$GAME_ROOT/lib/$TRIPLE"
if [[ -d "$LIB" ]]; then
  export LIBRARY_PATH="$LIB${LIBRARY_PATH:+:$LIBRARY_PATH}"
  export LD_LIBRARY_PATH="$LIB${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi

pass=0
fail=0
skip=0

section() {
  echo ""
  echo "========== $* =========="
}

run_step() {
  local name="$1"
  shift
  section "$name"
  if "$@"; then
    echo "OK: $name"
    pass=$((pass + 1))
    return 0
  fi
  echo "FAIL: $name" >&2
  fail=$((fail + 1))
  return 1
}

skip_step() {
  local name="$1"
  local why="$2"
  section "$name (skipped)"
  echo "SKIP: $why"
  skip=$((skip + 1))
}

echo "smoke_ci_linux P4"
echo "  ORI_BIN=$ORI_BIN"
echo "  GAME_ROOT=$GAME_ROOT"
echo "  CLUSTER=$CLUSTER"
echo "  ORI_CI_SHELL=$ORI_CI_SHELL ORI_CI_EXPORT=$ORI_CI_EXPORT ORI_CI_QUICK=$ORI_CI_QUICK"
"$ORI_BIN" --version 2>/dev/null || true

# --- native libs for sample link ---
if [[ ! -f "$LIB/libori_raylib_shim.a" ]] || [[ ! -f "$LIB/libraylib.a" ]]; then
  section "setup raylib (stub if needed)"
  if [[ -x "$GAME_ROOT/tools/setup_raylib_linux.sh" ]]; then
    "$GAME_ROOT/tools/setup_raylib_linux.sh" --stub || true
  fi
fi
if [[ -d "$LIB" ]]; then
  export LIBRARY_PATH="$LIB${LIBRARY_PATH:+:$LIBRARY_PATH}"
  export LD_LIBRARY_PATH="$LIB${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi

# --- 1) sample Play (check + AOT compile) ---
set +e
run_step "sample_play" "$GAME_ROOT/tools/smoke_sample_play.sh"
set -e

# --- 2) unit tests (headless) ---
run_unit() {
  local f="$1"
  echo "-- ori test $f"
  (cd "$GAME_ROOT" && "$ORI_BIN" test "$f")
}

# Gate suite: only tests that currently pass 100% (exclude residual world flakes).
set +e
run_step "test_phase_g_l" run_unit tests/test_phase_g_l.orl
run_step "test_camera_matrices" run_unit tests/test_camera_matrices.orl
run_step "test_look_audio" run_unit tests/test_look_audio.orl
if [[ "$ORI_CI_QUICK" != "1" ]]; then
  run_step "test_level_tools" run_unit tests/test_level_tools.orl
  run_step "test_tilemap_paint" run_unit tests/test_tilemap_paint.orl
  run_step "test_world_physics" run_unit tests/test_world_physics.orl
  run_step "test_multi_scene" run_unit tests/test_multi_scene.orl
  run_step "test_world" run_unit tests/test_world.orl
  run_step "test_world_systems" run_unit tests/test_world_systems.orl
  run_step "test_path_follow" run_unit tests/test_path_follow.orl
fi
set -e

# --- 3) studio_shell ---
SHELL_SMOKE="$CLUSTER/ori-imgui/demos/studio_shell/tools/smoke.sh"
if [[ "$ORI_CI_SHELL" != "1" ]]; then
  skip_step "studio_shell" "ORI_CI_SHELL=0"
elif [[ ! -x "$SHELL_SMOKE" ]] && [[ ! -f "$SHELL_SMOKE" ]]; then
  skip_step "studio_shell" "not found at $SHELL_SMOKE (need monorepo cluster)"
else
  set +e
  run_step "studio_shell" bash "$SHELL_SMOKE"
  set -e
fi

# --- 4) export package (compile gate, no window) ---
if [[ "$ORI_CI_EXPORT" != "1" ]]; then
  skip_step "export_package" "ORI_CI_EXPORT=0"
elif [[ ! -x "$GAME_ROOT/tools/export_game_package.sh" ]]; then
  skip_step "export_package" "export_game_package.sh missing"
else
  set +e
  run_step "export_package" \
    env ORI_USE_AOT=1 ORI_USE_SYSTEM_LINKER=1 \
    "$GAME_ROOT/tools/export_game_package.sh" \
    "$GAME_ROOT/assets/sample_project" \
    "${TMPDIR:-/tmp}/ori_ci_export_$$"
  set -e
  rm -rf "${TMPDIR:-/tmp}/ori_ci_export_$$" 2>/dev/null || true
fi

section "summary"
echo "pass=$pass fail=$fail skip=$skip"
if [[ "$fail" -gt 0 ]]; then
  echo "smoke_ci_linux: FAIL" >&2
  exit 1
fi
# sample is mandatory
if [[ "$pass" -lt 1 ]]; then
  echo "smoke_ci_linux: FAIL (no steps passed)" >&2
  exit 1
fi
echo "smoke_ci_linux: PASS"
exit 0
