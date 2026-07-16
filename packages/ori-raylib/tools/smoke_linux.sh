#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
"$root/tools/setup_raylib_linux.sh" --stub
"$ORI_BIN" check "$root/raylib.orl"
# Minimal program that only typechecks L0 symbols (no window needed for check)
cat > /tmp/ori_raylib_smoke.orl << 'EOF'
module app.raylib_smoke
import raylib = rl
import ori.io = io
main()
    -- link resolves ori_rl_* from stub
    io.print("ok")
end
EOF
out=$(mktemp)
# Compile from package root so path dep / local module resolve
cd "$root"
# standalone file next to package: copy to examples
mkdir -p "$root/examples"
cp /tmp/ori_raylib_smoke.orl "$root/examples/smoke.orl"
"$ORI_BIN" compile "$root/examples/smoke.orl" --out "$out"
"$out" | tee /tmp/ori_raylib_smoke.txt
rm -f "$out"
grep -qx ok /tmp/ori_raylib_smoke.txt
echo "smoke_raylib: all green"
