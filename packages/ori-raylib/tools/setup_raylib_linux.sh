#!/usr/bin/env sh
# Stage native libs for ori-raylib (and consumers that copy from lib/):
#   libori_raylib_shim.a  — ori_rl_* scalar ABI (stub or real wrappers)
#   libraylib.a           — real raylib, or tiny dummy when --stub
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out_dir="$repo_root/lib/$triple"
mkdir -p "$out_dir"

shim_a="$out_dir/libori_raylib_shim.a"
ray_a="$out_dir/libraylib.a"

force_stub=0
for arg in "$@"; do
    case "$arg" in
        --stub) force_stub=1 ;;
        -h|--help)
            echo "Usage: tools/setup_raylib_linux.sh [--stub]"
            exit 0
            ;;
    esac
done

build_stub_shim() {
    echo "building headless ori_rl_* stub → $shim_a"
    tmp=$(mktemp -d)
    cc -c -O2 -fPIC -o "$tmp/raylib_stub.o" "$script_dir/raylib_stub.c"
    rm -f "$shim_a"
    ar rcs "$shim_a" "$tmp/raylib_stub.o"
    # Dummy libraylib so native_libs = [shim, raylib] always resolves.
    printf 'void ori_raylib_dummy(void) {}\n' > "$tmp/dummy.c"
    cc -c -O2 -fPIC -o "$tmp/dummy.o" "$tmp/dummy.c"
    rm -f "$ray_a"
    ar rcs "$ray_a" "$tmp/dummy.o"
    rm -rf "$tmp"
    echo "stub installed (WindowShouldClose exits after a few frames; no X11)."
}

try_system_raylib() {
    for candidate in \
        /usr/lib/x86_64-linux-gnu/libraylib.a \
        /usr/lib/libraylib.a \
        /usr/local/lib/libraylib.a
    do
        if [ -f "$candidate" ]; then
            cp -f "$candidate" "$ray_a"
            echo "copied system raylib: $candidate"
            return 0
        fi
    done
    return 1
}

try_build_raylib_source() {
    if ! command -v cc >/dev/null 2>&1; then
        return 1
    fi
    if [ ! -f /usr/include/X11/Xlib.h ]; then
        echo "X11 headers missing; cannot build desktop raylib from source."
        return 1
    fi
    if [ ! -f /usr/include/raylib.h ] && [ ! -f /usr/local/include/raylib.h ]; then
        # headers come with the source tree
        :
    fi
    src_root="${TMPDIR:-/tmp}/ori-raylib-src-$$"
    mkdir -p "$src_root"
    if ! curl -sL "https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz" \
        | tar xz -C "$src_root"; then
        rm -rf "$src_root"
        return 1
    fi
    (
        cd "$src_root/raylib-5.5/src"
        make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC -j"$(nproc 2>/dev/null || echo 2)"
        cp -f libraylib.a "$ray_a"
        cp -f raylib.h "$out_dir/raylib.h" 2>/dev/null || true
    )
    status=$?
    rm -rf "$src_root"
    return $status
}

build_real_shim() {
    include_flags=""
    for inc in /usr/include /usr/local/include "$out_dir"; do
        if [ -f "$inc/raylib.h" ]; then
            include_flags="-I$inc"
            break
        fi
    done
    if [ -z "$include_flags" ]; then
        echo "raylib.h not found; cannot build real shim" >&2
        return 1
    fi
    echo "building real ori_raylib_shim → $shim_a ($include_flags)"
    tmp=$(mktemp -d)
    cc -c -O2 -fPIC $include_flags -o "$tmp/shim.o" "$repo_root/native/ori_raylib_shim.c"
    rm -f "$shim_a"
    ar rcs "$shim_a" "$tmp/shim.o"
    rm -rf "$tmp"
}

if [ "$force_stub" -eq 1 ]; then
    build_stub_shim
    exit 0
fi

if try_system_raylib || try_build_raylib_source; then
    if build_real_shim; then
        echo "real raylib + shim ready → $out_dir"
        exit 0
    fi
    echo "real raylib present but shim failed; falling back to stub"
fi

echo "falling back to headless stub"
build_stub_shim
