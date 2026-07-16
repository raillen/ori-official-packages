#!/usr/bin/env bash
# Smoke: package resolve + ori check for Studio sample (engine_test).
# See docs/planning/PLAY-PACKAGING.md
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SAMPLE="$ROOT/assets/sample_project"
export ORI_GAME_ROOT="$ROOT"
TRIPLE="${ORI_HOST_TRIPLE:-x86_64-unknown-linux-gnu}"
LIB="$ROOT/lib/$TRIPLE"
if [[ -d "$LIB" ]]; then
  export LIBRARY_PATH="$LIB${LIBRARY_PATH:+:$LIBRARY_PATH}"
  export LD_LIBRARY_PATH="$LIB${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
  echo "native lib dir OK: $LIB"
else
  echo "WARN: native lib dir missing: $LIB (Play may fail at link/run)"
fi
echo "== smoke_sample_play =="
echo "sample=$SAMPLE"
echo "ORI_GAME_ROOT=$ORI_GAME_ROOT"
test -f "$SAMPLE/ori.pkg.toml"
test -f "$SAMPLE/main.orl"
test -f "$SAMPLE/scenes/engine_test.scene.json"
test -f "$SAMPLE/assets/models/brazilian-cowboy.glb"
grep -q 'ori_game' "$SAMPLE/ori.pkg.toml"
grep -q 'path' "$SAMPLE/ori.pkg.toml"
cd "$SAMPLE"
ori check .
echo "ori check . OK"
# AOT compile (Play path). JIT needs .so not staged.
export ORI_USE_AOT=1
OUT="${TMPDIR:-/tmp}/ori_smoke_sample_$$"
ori compile . -o "$OUT" 
echo "ori compile . OK ($OUT)"
# main must not be empty: binary linked; full window run is interactive.
if ! grep -q 'run_window' main.orl; then
  echo "FAIL: main.orl must call game.app.run_window" >&2
  exit 1
fi
rm -f "$OUT" "$OUT.o" 2>/dev/null || true
echo "smoke_sample_play: PASS"
