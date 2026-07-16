#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
enet="$root/vendor/enet"
mkdir -p "$out"

if [ ! -f "$enet/include/enet/enet.h" ]; then
    echo "missing vendor/enet (git clone https://github.com/lsalzman/enet.git vendor/enet)" >&2
    exit 1
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

incs="-I$enet/include"
objs=""
for src in callbacks.c compress.c host.c list.c packet.c peer.c protocol.c unix.c; do
    o="$tmp/${src%.c}.o"
    cc -c -O2 -fPIC $incs -o "$o" "$enet/$src"
    objs="$objs $o"
done
# shellcheck disable=SC2086
ar rcs "$out/libenet.a" $objs

cc -c -O2 -fPIC $incs -o "$tmp/ori_enet_shim.o" "$root/native/ori_enet_shim.c"
ar rcs "$out/libori_enet_shim.a" "$tmp/ori_enet_shim.o"

echo "built enet + shim → $out"
ls -la "$out"
