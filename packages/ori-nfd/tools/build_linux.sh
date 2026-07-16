#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"
c++ -O2 -fPIC -I"$root/vendor" -std=c++17 \
  -c -o /tmp/ori_nfd_shim.o "$root/native/ori_nfd_shim.cpp"
# Real static objects (for ld script)
ar rcs "$out/libori_nfd_shim_objs.a" /tmp/ori_nfd_shim.o
# Shared for JIT (C++ runtime linked in)
c++ -shared -O2 -fPIC -I"$root/vendor" -std=c++17 \
  -o "$out/libori_nfd_shim.so" "$root/native/ori_nfd_shim.cpp" -lstdc++ -lpthread
# AOT: GNU ld script named as the package native_lib
abs=$(CDPATH= cd -- "$out" && pwd)
printf '/* GNU ld script */\nINPUT ( %s/libori_nfd_shim_objs.a -lstdc++ -lpthread )\n' "$abs" \
  > "$out/libori_nfd_shim.a"
echo "built nfd → $out"
ls -la "$out"
