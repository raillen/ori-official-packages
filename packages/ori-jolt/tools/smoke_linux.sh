#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"

# Prefer stub for fast CI; set ORI_JOLT_REAL=1 to build real Jolt.
if [ "${ORI_JOLT_REAL:-0}" = "1" ]; then
  "$root/tools/build_linux.sh"
else
  "$root/tools/build_linux.sh" --stub
fi

"$ORI_BIN" check "$root/jolt/units.orl"
"$ORI_BIN" check "$root/jolt/dynamics.orl"
"$ORI_BIN" check "$root/jolt/character_ctrl.orl"
"$ORI_BIN" check "$root/jolt/world_character.orl"
"$ORI_BIN" check "$root/jolt/debug_draw.orl"

for ex in boxes_fall impulse_test constraint_test character_test vehicle_test character_frame_test vehicle_modes_test wave4_test character_world_test; do
  out=$(mktemp)
  if [ "${ORI_JOLT_REAL:-0}" = "1" ]; then
    ORI_USE_SYSTEM_LINKER=1 ORI_NATIVE_LINKER="${ORI_NATIVE_LINKER:-g++}" \
      "$ORI_BIN" compile "$root/examples/${ex}.orl" --out "$out"
  else
    "$ORI_BIN" compile "$root/examples/${ex}.orl" --out "$out"
  fi
  "$out" | tee "/tmp/ori_jolt_${ex}.txt"
  rm -f "$out"
done

grep -q fell /tmp/ori_jolt_boxes_fall.txt
grep -q moved /tmp/ori_jolt_impulse_test.txt
grep -q constrained /tmp/ori_jolt_constraint_test.txt
grep -q walked /tmp/ori_jolt_character_test.txt
grep -q drove /tmp/ori_jolt_vehicle_test.txt
grep -q framed /tmp/ori_jolt_character_frame_test.txt
grep -q modes_ok /tmp/ori_jolt_vehicle_modes_test.txt
grep -q wave4_ok /tmp/ori_jolt_wave4_test.txt
grep -q world_char_ok /tmp/ori_jolt_character_world_test.txt
echo "smoke_jolt: all green"
