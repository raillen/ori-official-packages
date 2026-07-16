#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
"$root/tools/build_linux.sh"
cc -O2 -I"$root/vendor" -include stdint.h -o "$root/tools/pack_cli" "$root/tools/pack_cli.c"
"$ORI_BIN" check "$root/rres/pack.orl"
cd "$root"
out=$(mktemp)
"$ORI_BIN" compile examples/pack_roundtrip.orl --out "$out"
"$out" | tee /tmp/ori_rres_smoke.txt
grep -qx ok /tmp/ori_rres_smoke.txt
rm -f "$out"
"$root/tools/pack_cli" /tmp/cli_smoke.orpk hello.txt=examples/assets/hello.txt >/dev/null
echo "smoke_rres: all green"
