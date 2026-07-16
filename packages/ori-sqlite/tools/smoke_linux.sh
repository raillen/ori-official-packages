#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
"$root/tools/build_linux.sh"
"$ORI_BIN" check "$root/sqlite/db.orl"
for ex in kv_store tx_rollback prepared_demo; do
  out=$(mktemp)
  "$ORI_BIN" compile "$root/examples/${ex}.orl" --out "$out"
  "$out" | tee "/tmp/ori_sql_${ex}.txt"
  rm -f "$out"
  grep -qx ok "/tmp/ori_sql_${ex}.txt"
done
echo "smoke_sqlite: all green"
