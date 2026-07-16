#!/usr/bin/env sh
# P3 — macOS smoke for ori-game (check + unit gate + sample compile).
# Requires: ori with darwin runtime staged; ORI_USE_SYSTEM_LINKER=1.
#
#   export ORI_BIN=ori ORI_USE_SYSTEM_LINKER=1
#   ./tools/smoke_macos.sh
#
# Detects host triple (x86_64 / aarch64 apple-darwin). Does not require
# pre-staged raylib when only `ori check` / `ori test` / sample package check
# are enough; full window demos need native libs under lib/<triple>/.
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)

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
export ORI_GAME_ROOT="$repo_root"

detect_triple() {
    if [ -n "${ORI_HOST_TRIPLE:-}" ]; then
        echo "$ORI_HOST_TRIPLE"
        return
    fi
    arch=$(uname -m 2>/dev/null || echo x86_64)
    case "$arch" in
        arm64|aarch64) echo "aarch64-apple-darwin" ;;
        x86_64|amd64) echo "x86_64-apple-darwin" ;;
        *) echo "${arch}-apple-darwin" ;;
    esac
}

TRIPLE=$(detect_triple)
export ORI_HOST_TRIPLE="$TRIPLE"
LIB="$repo_root/lib/$TRIPLE"
echo "== smoke_macos (ori-game) =="
echo "triple=$TRIPLE"
echo "lib=$LIB"

if [ -d "$LIB" ]; then
    export LIBRARY_PATH="$LIB${LIBRARY_PATH:+:$LIBRARY_PATH}"
    export DYLD_LIBRARY_PATH="$LIB${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"
    echo "native lib dir OK"
else
    echo "WARN: no staged libs at $LIB — compile of windowed demos may fail"
fi

echo "== ori check core modules =="
fail=0
for f in \
    "$repo_root/game/app.orl" \
    "$repo_root/game/world.orl" \
    "$repo_root/game/ogame_level_csg.orl" \
    "$repo_root/game/play_freshness.orl" \
    "$repo_root/game/character_controller.orl"
do
    if [ -f "$f" ]; then
        if "$ORI_BIN" check "$f"; then
            echo "OK check $(basename "$f")"
        else
            echo "FAIL check $f"
            fail=$((fail + 1))
        fi
    fi
done

echo "== unit gate (subset) =="
for t in \
    test_engine.orl \
    test_world.orl \
    test_play_freshness.orl \
    test_csg_l6.orl \
    test_phase_g_l.orl
do
    path="$repo_root/tests/$t"
    if [ -f "$path" ]; then
        if "$ORI_BIN" test "$path"; then
            echo "OK test $t"
        else
            echo "FAIL test $t"
            fail=$((fail + 1))
        fi
    else
        echo "SKIP test $t (missing)"
    fi
done

echo "== sample package check =="
SAMPLE="$repo_root/assets/sample_project"
if [ -f "$SAMPLE/ori.pkg.toml" ]; then
    if (CDPATH= cd -- "$SAMPLE" && "$ORI_BIN" check .); then
        echo "OK sample ori check ."
    else
        echo "FAIL sample check"
        fail=$((fail + 1))
    fi
else
    echo "SKIP sample (missing)"
fi

if [ "$fail" -gt 0 ]; then
    echo "smoke_macos: FAILED ($fail)"
    exit 1
fi
echo "smoke_macos: all green"
exit 0
