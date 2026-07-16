#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"$root/tools/build_linux.sh"

# Fixtures for smoke + unit tests (temp dirs)
rm -rf /tmp/ori_physfs_test /tmp/ori_physfs_test_b /tmp/ori_physfs_write
mkdir -p /tmp/ori_physfs_test /tmp/ori_physfs_test_b /tmp/ori_physfs_write
printf 'hello-physfs\n' > /tmp/ori_physfs_test/hello.txt
printf 'from-b\n' > /tmp/ori_physfs_test_b/other.txt

ORI_BIN="${ORI_BIN:-ori}"
export ORI_USE_SYSTEM_LINKER="${ORI_USE_SYSTEM_LINKER:-1}"
export ORI_USE_JIT="${ORI_USE_JIT:-1}"
export LD_LIBRARY_PATH="$root/lib/x86_64-unknown-linux-gnu:${LD_LIBRARY_PATH:-}"

"$ORI_BIN" check "$root/physfs/fs.orl"
"$ORI_BIN" check "$root/examples/smoke_physfs.orl"
"$ORI_BIN" check "$root/tests/test_physfs.orl"
out=$("$ORI_BIN" run "$root/examples/smoke_physfs.orl")
echo "$out"
echo "$out" | grep -qx ok
"$ORI_BIN" test "$root/tests/test_physfs.orl"
echo "smoke ok"
