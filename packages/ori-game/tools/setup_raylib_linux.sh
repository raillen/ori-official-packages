#!/usr/bin/env sh
# Compatibility wrapper: stage raylib L0 into ori-game/lib via sibling ori-raylib.
set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
game_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
ray_root=$(CDPATH= cd -- "$game_root/../ori-raylib" && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"

if [ ! -d "$ray_root" ]; then
    echo "missing sibling ori-raylib at $ray_root" >&2
    exit 1
fi

# Forward args (--stub, etc.)
"$ray_root/tools/setup_raylib_linux.sh" "$@"

# Copy staged artifacts so ori-game native_libs resolve under game/lib/
mkdir -p "$game_root/lib/$triple"
cp -f "$ray_root/lib/$triple/libori_raylib_shim.a" "$game_root/lib/$triple/"
cp -f "$ray_root/lib/$triple/libraylib.a" "$game_root/lib/$triple/"
if [ -f "$ray_root/lib/$triple/raylib.h" ]; then
    cp -f "$ray_root/lib/$triple/raylib.h" "$game_root/lib/$triple/"
fi
echo "ori-game: staged raylib L0 from ori-raylib → lib/$triple"
