#!/usr/bin/env bash
# P2 — Export / package a game project (AOT bin + assets + native_libs).
# Usage:
#   ./tools/export_game_package.sh [project_dir] [out_dir]
# Defaults: sample_project → /tmp/ori_game_export_<name>
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CLUSTER="$(cd "$ROOT/.." && pwd)"
PROJECT="${1:-$ROOT/assets/sample_project}"
NAME="$(basename "$PROJECT")"
OUT="${2:-/tmp/ori_game_export_${NAME}}"
# Default triple: honor ORI_HOST_TRIPLE, else detect (Linux / Darwin).
if [ -z "${ORI_HOST_TRIPLE:-}" ]; then
  case "$(uname -s 2>/dev/null)" in
    Darwin)
      case "$(uname -m 2>/dev/null)" in
        arm64|aarch64) ORI_HOST_TRIPLE="aarch64-apple-darwin" ;;
        *) ORI_HOST_TRIPLE="x86_64-apple-darwin" ;;
      esac
      ;;
    *)
      ORI_HOST_TRIPLE="x86_64-unknown-linux-gnu"
      ;;
  esac
fi
TRIPLE="$ORI_HOST_TRIPLE"
ORI_BIN="${ORI_BIN:-ori}"

if [[ ! -f "$PROJECT/ori.pkg.toml" ]]; then
  echo "FAIL: no ori.pkg.toml in $PROJECT" >&2
  exit 1
fi
if [[ ! -f "$PROJECT/main.orl" ]]; then
  echo "FAIL: no main.orl in $PROJECT" >&2
  exit 1
fi

export ORI_GAME_ROOT="$ROOT"
export ORI_USE_AOT=1
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
LIB="$ROOT/lib/$TRIPLE"
if [[ -d "$LIB" ]]; then
  export LIBRARY_PATH="$LIB${LIBRARY_PATH:+:$LIBRARY_PATH}"
  export LD_LIBRARY_PATH="$LIB${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
fi

echo "== export_game_package =="
echo "project=$PROJECT"
echo "out=$OUT"
echo "triple=$TRIPLE"

rm -rf "$OUT"
mkdir -p "$OUT/bin" "$OUT/native" "$OUT/assets" "$OUT/scenes" "$OUT/entities"

echo "-- ori check"
(cd "$PROJECT" && "$ORI_BIN" check .)

echo "-- ori compile"
BIN_NAME="$NAME"
(cd "$PROJECT" && "$ORI_BIN" compile . -o "$OUT/bin/$BIN_NAME")

# Content trees (best-effort)
if [[ -d "$PROJECT/assets" ]]; then
  cp -a "$PROJECT/assets/." "$OUT/assets/"
fi
if [[ -d "$PROJECT/scenes" ]]; then
  cp -a "$PROJECT/scenes/." "$OUT/scenes/"
fi
if [[ -d "$PROJECT/entities" ]]; then
  cp -a "$PROJECT/entities/." "$OUT/entities/"
fi
# Manifests useful at runtime
for f in ori.pkg.toml project.ogame.toml README.md; do
  if [[ -f "$PROJECT/$f" ]]; then
    cp -a "$PROJECT/$f" "$OUT/"
  fi
done

# Native libs from ori-game + raylib path-dep stage
if [[ -d "$LIB" ]]; then
  cp -a "$LIB/." "$OUT/native/" 2>/dev/null || true
fi
RAYLIB_LIB="$CLUSTER/ori-raylib/lib/$TRIPLE"
if [[ -d "$RAYLIB_LIB" ]]; then
  cp -a "$RAYLIB_LIB/." "$OUT/native/" 2>/dev/null || true
fi

cat > "$OUT/run.sh" << EOF
#!/usr/bin/env bash
# Run exported package. Binary expects assets relative to cwd = package root.
set -euo pipefail
DIR="\$(cd "\$(dirname "\$0")" && pwd)"
cd "\$DIR"
export LIBRARY_PATH="\$DIR/native\${LIBRARY_PATH:+:\$LIBRARY_PATH}"
case "\$(uname -s 2>/dev/null)" in
  Darwin)
    export DYLD_LIBRARY_PATH="\$DIR/native\${DYLD_LIBRARY_PATH:+:\$DYLD_LIBRARY_PATH}"
    ;;
  *)
    export LD_LIBRARY_PATH="\$DIR/native\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
    ;;
esac
export ORI_USE_AOT=1
exec "\$DIR/bin/$BIN_NAME" "\$@"
EOF
chmod +x "$OUT/run.sh"

cat > "$OUT/EXPORT.md" << EOF
# Ori game export

- **Project:** \`$NAME\`
- **Host triple:** \`$TRIPLE\`
- **Binary:** \`bin/$BIN_NAME\`
- **Run:** \`./run.sh\` (from this directory)

Includes scenes/entities/assets when present, plus staged \`native/\` libs
from ori-game / ori-raylib for this triple.

Not a full installer: no icon, no multi-OS fat package. See
\`docs/planning/PLAY-PACKAGING.md\` and PHASE-OS.
EOF

echo "export OK: $OUT"
echo "  bin: $OUT/bin/$BIN_NAME"
ls -la "$OUT/bin/$BIN_NAME"
test -x "$OUT/bin/$BIN_NAME" || test -f "$OUT/bin/$BIN_NAME"
echo "smoke: (cd $OUT && ./run.sh)  # interactive window"
