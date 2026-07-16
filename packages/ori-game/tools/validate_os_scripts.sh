#!/usr/bin/env sh
# P1/P3 polish — validate multi-OS smoke scaffolding from any host (no MSVC/mac needed).
# Checks that PHASE-OS core packages have Windows scripts and that macOS umbrellas exist.
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
game_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
proj_root=$(CDPATH= cd -- "$game_root/.." && pwd)

ok=0
fail=0
warn=0

check_file() {
    label=$1
    path=$2
    if [ -f "$path" ]; then
        echo "OK   $label"
        ok=$((ok + 1))
    else
        echo "FAIL $label — missing $path"
        fail=$((fail + 1))
    fi
}

warn_missing() {
    label=$1
    path=$2
    if [ -f "$path" ]; then
        echo "OK   $label"
        ok=$((ok + 1))
    else
        echo "WARN $label — missing $path (optional)"
        warn=$((warn + 1))
    fi
}

echo "== validate_os_scripts (P1 Windows + P3 macOS scaffolding) =="
echo "proj_root=$proj_root"

# Umbrellas
check_file "P1 umbrella" "$game_root/tools/smoke_eco_windows.ps1"
check_file "P3 umbrella" "$game_root/tools/smoke_eco_macos.sh"
check_file "P3 game smoke" "$game_root/tools/smoke_macos.sh"
check_file "P1 game smoke" "$game_root/tools/smoke_windows.ps1"
check_file "Linux umbrella" "$game_root/tools/smoke_eco_linux.sh"
check_file "PHASE-OS doc" "$game_root/docs/planning/PHASE-OS.md"

# Core 8 Windows smoke scripts (PHASE-OS)
for pkg in ori-game ori-box2d ori-jolt ori-sqlite ori-rres ori-imgui ori-raygui ori-enet; do
    if [ "$pkg" = "ori-game" ]; then
        check_file "$pkg smoke_windows" "$game_root/tools/smoke_windows.ps1"
    else
        check_file "$pkg smoke_windows" "$proj_root/$pkg/tools/smoke_windows.ps1"
    fi
done

# Broken-ref guards (module renames)
if grep -q 'jolt\\world\.orl\|jolt/world\.orl' "$proj_root/ori-jolt/tools/smoke_windows.ps1" 2>/dev/null; then
    echo "FAIL ori-jolt smoke_windows still checks jolt/world.orl (use dynamics.orl)"
    fail=$((fail + 1))
else
    echo "OK   ori-jolt smoke_windows module path"
    ok=$((ok + 1))
fi

if grep -q 'box2d\\world\.orl\|box2d/world\.orl' "$proj_root/ori-box2d/tools/smoke_windows.ps1" 2>/dev/null; then
    echo "FAIL ori-box2d smoke_windows still checks box2d/world.orl (use dynamics.orl)"
    fail=$((fail + 1))
else
    echo "OK   ori-box2d smoke_windows module path"
    ok=$((ok + 1))
fi

# Studio-critical optional Win scripts
for pkg in ori-nfd ori-imguizmo ori-imnodes ori-implot; do
    warn_missing "$pkg smoke_windows" "$proj_root/$pkg/tools/smoke_windows.ps1"
done

# Executable bits on sh scripts
for sh in smoke_macos.sh smoke_eco_macos.sh validate_os_scripts.sh smoke_eco_linux.sh; do
    path="$game_root/tools/$sh"
    if [ -f "$path" ] && [ -x "$path" ]; then
        echo "OK   executable $sh"
        ok=$((ok + 1))
    elif [ -f "$path" ]; then
        echo "WARN not executable: $sh (chmod +x recommended)"
        warn=$((warn + 1))
    fi
done

echo ""
echo "validate_os_scripts: ok=$ok fail=$fail warn=$warn"
if [ "$fail" -gt 0 ]; then
    echo "validate_os_scripts: FAILED"
    exit 1
fi
echo "validate_os_scripts: PASS"
exit 0
