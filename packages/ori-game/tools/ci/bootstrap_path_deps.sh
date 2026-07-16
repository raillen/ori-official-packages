#!/usr/bin/env bash
# Create minimal sibling packages for ori_game path-deps when missing (CI only).
# Expects cwd = ori-game package root. Siblings live in parent directory.
set -euo pipefail

GAME_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PARENT="$(cd "$GAME_ROOT/.." && pwd)"
TRIPLE="${ORI_HOST_TRIPLE:-x86_64-unknown-linux-gnu}"

echo "bootstrap_path_deps: parent=$PARENT"

# Minimal package: name, version, optional entry module text.
write_stub_pkg() {
  local dir="$1"
  local name="$2"
  local version="$3"
  local entry="${4:-main.orl}"
  local entry_body="${5:--- stub package for CI path-dep resolve\nmodule ci_stub.$name\n}"
  if [[ -f "$dir/ori.pkg.toml" ]]; then
    echo "  exists: $dir"
    return 0
  fi
  echo "  create stub: $dir ($name $version)"
  mkdir -p "$dir"
  cat > "$dir/ori.pkg.toml" << EOF
[package]
name = "$name"
version = "$version"
entry = "$entry"
ori_version = "0.3.0"
description = "CI stub path-dep (bootstrap_path_deps.sh) — not for production"
EOF
  # entry file
  local entry_path="$dir/$entry"
  mkdir -p "$(dirname "$entry_path")"
  if [[ ! -f "$entry_path" ]]; then
    printf '%b\n' "$entry_body" > "$entry_path"
  fi
}

# raylib L0 — real layout if missing (shim libs staged later by setup_raylib_linux).
if [[ ! -f "$PARENT/ori-raylib/ori.pkg.toml" ]]; then
  echo "  bootstrap ori-raylib"
  mkdir -p "$PARENT/ori-raylib/lib/$TRIPLE" "$PARENT/ori-raylib/native" "$PARENT/ori-raylib/tools"
  cat > "$PARENT/ori-raylib/ori.pkg.toml" << EOF
[package]
name = "raylib"
version = "0.1.0"
entry = "raylib.orl"
ori_version = "0.3.0"
description = "CI bootstrap raylib L0"
native_libs = ["ori_raylib_shim", "raylib"]
EOF
  # Prefer copy from game-engine-full tree if this script is run inside monorepo
  # with raylib only missing (rare). Else minimal extern surface.
  if [[ -f "$GAME_ROOT/raylib.orl" ]]; then
    # ori-game keeps a historical copy? use stub module
    :
  fi
  if [[ -f "$PARENT/ori-raylib/raylib.orl" ]]; then
    :
  elif [[ -f "$GAME_ROOT/../ori-raylib/raylib.orl" ]]; then
    cp -a "$GAME_ROOT/../ori-raylib/raylib.orl" "$PARENT/ori-raylib/" 2>/dev/null || true
  fi
  if [[ ! -f "$PARENT/ori-raylib/raylib.orl" ]]; then
    # Minimal module so package resolves; real smoke uses staged shim from ori-game/lib.
    cat > "$PARENT/ori-raylib/raylib.orl" << 'EOF'
module raylib

-- CI bootstrap stub: real bindings live in monorepo ori-raylib.
-- Sample/tests that only use game.* pure modules do not call these.

extern c
    public ori_rl_GetScreenWidth() -> int
    public ori_rl_GetScreenHeight() -> int
end
EOF
  fi
  # Point native libs at game-root stage after setup_raylib_linux
  mkdir -p "$PARENT/ori-raylib/lib/$TRIPLE"
fi

# Optional ECO path-deps from ori.pkg.toml (manifest only is enough for resolve
# if the sample does not import them). Versions match ori-game/ori.pkg.toml.
write_stub_pkg "$PARENT/ori-rres" "rres" "0.3.0" "rres.orl" "module rres\n"
write_stub_pkg "$PARENT/ori-freetype" "freetype" "0.1.0" "freetype.orl" "module freetype\n"
write_stub_pkg "$PARENT/ori-cgltf" "cgltf" "0.2.0" "cgltf.orl" "module cgltf\n"
write_stub_pkg "$PARENT/ori-fast-obj" "fast_obj" "0.2.0" "fast_obj.orl" "module fast_obj\n"
write_stub_pkg "$PARENT/ori-physfs" "physfs" "0.2.0" "physfs.orl" "module physfs\n"
write_stub_pkg "$PARENT/ori-noise" "noise" "0.2.0" "noise.orl" "module noise\n"
write_stub_pkg "$PARENT/ori-lz4" "lz4" "0.2.0" "lz4.orl" "module lz4\n"
write_stub_pkg "$PARENT/ori-miniz" "miniz" "0.2.0" "miniz.orl" "module miniz\n"
write_stub_pkg "$PARENT/ori-recast" "recast" "0.2.0" "recast.orl" "module recast\n"

# Mirror game-staged libs into ori-raylib if setup already ran
if [[ -d "$GAME_ROOT/lib/$TRIPLE" ]]; then
  mkdir -p "$PARENT/ori-raylib/lib/$TRIPLE"
  cp -f "$GAME_ROOT/lib/$TRIPLE/"*.a "$PARENT/ori-raylib/lib/$TRIPLE/" 2>/dev/null || true
fi

echo "bootstrap_path_deps: done"
