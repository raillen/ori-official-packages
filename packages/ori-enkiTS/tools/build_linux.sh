#!/usr/bin/env sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
V="$root/vendor"
mkdir -p "$out"
c++ -O2 -fPIC -I"$V" -c -o /tmp/TaskScheduler.o "$V/TaskScheduler.cpp"
c++ -O2 -fPIC -I"$V" -c -o /tmp/TaskScheduler_c.o "$V/TaskScheduler_c.cpp"
cc -O2 -fPIC -I"$V" -c -o /tmp/ori_enkits_shim.o "$root/native/ori_enkits_shim.c"
# Real objects archive
ar rcs "$out/libori_enkits_objs.a" /tmp/ori_enkits_shim.o /tmp/TaskScheduler.o /tmp/TaskScheduler_c.o
# Shared for JIT
c++ -shared -O2 -fPIC -o "$out/libori_enkits_shim.so" \
  /tmp/ori_enkits_shim.o /tmp/TaskScheduler.o /tmp/TaskScheduler_c.o \
  -lpthread -lstdc++
# AOT: ld script as package native_lib name
abs=$(CDPATH= cd -- "$out" && pwd)
printf '/* GNU ld script */\nINPUT ( %s/libori_enkits_objs.a -lstdc++ -lpthread )\n' "$abs" \
  > "$out/libori_enkits_shim.a"
echo "built enkits → $out"
nm -D "$out/libori_enkits_shim.so" | grep ' T ori_enki_init' || {
  echo "missing ori_enki_init" >&2
  exit 1
}
ls -la "$out"
