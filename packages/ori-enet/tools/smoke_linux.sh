#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
"$root/tools/build_linux.sh"
"$ORI_BIN" check "$root/enet/host.orl"
"$ORI_BIN" check "$root/enet/protocol.orl"
for ex in loopback_ping multi_message channels_demo broadcast_demo protocol_demo game_tf_sync tf_codec_smoke; do
  out=$(mktemp)
  "$ORI_BIN" compile "$root/examples/${ex}.orl" --out "$out"
  # Run from package root so cwd-relative packet temps resolve cleanly.
  (CDPATH= cd -- "$root" && "$out") | tee "/tmp/ori_enet_${ex}.txt"
  rm -f "$out"
  grep -qx ok "/tmp/ori_enet_${ex}.txt"
done
# Clean packet temps left in cwd
rm -f "$root"/ori_enet_*.tmp
echo "smoke_enet: all green"
