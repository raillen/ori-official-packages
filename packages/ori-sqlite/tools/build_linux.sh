#!/usr/bin/env sh
# Build static (.a) + shared (.so) artifacts for Linux GNU.
# AOT links .a; Ori JIT loads native_libs as libNAME.so from lib/<triple>/.
# Package declares: native_libs = ["ori_sqlite_shim", "sqlite3"]
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out" "$root/vendor"
if [ ! -f "$root/vendor/sqlite3.c" ]; then
  echo "missing vendor/sqlite3.c (amalgamation)" >&2
  exit 1
fi

CFLAGS="-O2 -fPIC -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_THREADSAFE=0"

echo "building static libs → $out"
cc -c $CFLAGS -o /tmp/sqlite3.o "$root/vendor/sqlite3.c"
ar rcs "$out/libsqlite3.a" /tmp/sqlite3.o
cc -c $CFLAGS -I"$root/vendor" -o /tmp/ori_sqlite_shim.o "$root/native/ori_sqlite_shim.c"
ar rcs "$out/libori_sqlite_shim.a" /tmp/ori_sqlite_shim.o

echo "building shared libraries (cdylib for JIT) → $out"
# sqlite3 shared first; shim links against it with $ORIGIN rpath
cc -shared $CFLAGS \
  -o "$out/libsqlite3.so" \
  "$root/vendor/sqlite3.c" \
  -lm

cc -shared $CFLAGS -I"$root/vendor" \
  -o "$out/libori_sqlite_shim.so" \
  "$root/native/ori_sqlite_shim.c" \
  -L"$out" -lsqlite3 -Wl,-rpath,'$ORIGIN' \
  -lm

echo "built sqlite → $out"
ls -la "$out"
