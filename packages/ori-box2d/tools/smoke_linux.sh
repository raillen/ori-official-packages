#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
"$root/tools/build_linux.sh"
"$ORI_BIN" check "$root/box2d/units.orl"
"$ORI_BIN" check "$root/box2d/world.orl"
"$ORI_BIN" check "$root/box2d/debug_draw.orl"
for ex in boxes_fall impulse_test joints_demo; do
  out=$(mktemp)
  "$ORI_BIN" compile "$root/examples/${ex}.orl" --out "$out"
  "$out" | tee "/tmp/ori_b2_${ex}.txt"
  rm -f "$out"
done
grep -q fell /tmp/ori_b2_boxes_fall.txt
grep -q moved /tmp/ori_b2_impulse_test.txt
grep -q joints_ok /tmp/ori_b2_joints_demo.txt
echo "smoke_box2d: all green"
