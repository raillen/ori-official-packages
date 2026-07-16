#!/usr/bin/env sh
# Build libori_jolt_shim.a
# Prefer real Jolt C++ shim; fall back to Euler stub (--stub or if Jolt build fails).
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
triple="${ORI_GAME_TRIPLE:-x86_64-unknown-linux-gnu}"
out="$root/lib/$triple"
mkdir -p "$out"

CMAKE_BIN="${CMAKE_BIN:-}"
if [ -z "$CMAKE_BIN" ]; then
    if [ -x /tmp/cmake-3.30.5-linux-x86_64/bin/cmake ]; then
        CMAKE_BIN=/tmp/cmake-3.30.5-linux-x86_64/bin/cmake
    elif command -v cmake >/dev/null 2>&1; then
        CMAKE_BIN=$(command -v cmake)
    fi
fi

force_stub=0
for arg in "$@"; do
    case "$arg" in
        --stub) force_stub=1 ;;
    esac
done

build_stub() {
    echo "building ori_jolt stub → $out/libori_jolt_shim.a"
    tmp=$(mktemp -d)
    cc -c -O2 -fPIC -o "$tmp/stub.o" "$root/native/ori_jolt_stub.c"
    rm -f "$out/libori_jolt_shim.a"
    ar rcs "$out/libori_jolt_shim.a" "$tmp/stub.o"
    # Dummy libJolt so native_libs = [shim, Jolt] always resolves.
    printf 'void ori_jolt_dummy_jolt(void) {}\n' > "$tmp/dummy.c"
    cc -c -O2 -fPIC -o "$tmp/dummy.o" "$tmp/dummy.c"
    rm -f "$out/libJolt.a"
    ar rcs "$out/libJolt.a" "$tmp/dummy.o"
    rm -rf "$tmp"
    echo "STUB=1" > "$out/ORI_JOLT_MODE.txt"
}

build_jolt_lib() {
    jolt_src="$root/vendor/JoltPhysics"
    if [ ! -d "$jolt_src/Jolt" ]; then
        echo "vendor/JoltPhysics missing" >&2
        return 1
    fi
    if [ -z "$CMAKE_BIN" ]; then
        echo "cmake not found" >&2
        return 1
    fi
    build_dir="$jolt_src/Build/ori-jolt-lib"
    if [ ! -f "$build_dir/libJolt.a" ] && [ ! -f "$build_dir/Release/libJolt.a" ]; then
        echo "configuring Jolt (this may take a few minutes)…"
        # Portable SIMD: AVX+SSE4 (no AVX2) — matches wider CPUs; override via cmake cache if needed.
        "$CMAKE_BIN" -S "$jolt_src/Build" -B "$build_dir" \
            -DCMAKE_BUILD_TYPE=Release \
            -DTARGET_UNIT_TESTS=OFF \
            -DTARGET_HELLO_WORLD=OFF \
            -DTARGET_PERFORMANCE_TEST=OFF \
            -DTARGET_SAMPLES=OFF \
            -DTARGET_VIEWER=OFF \
            -DINTERPROCEDURAL_OPTIMIZATION=OFF \
            -DCROSS_PLATFORM_DETERMINISTIC=OFF \
            -DUSE_AVX2=OFF -DUSE_AVX512=OFF -DUSE_FMADD=OFF \
            -DUSE_LZCNT=OFF -DUSE_TZCNT=OFF -DUSE_F16C=OFF \
            -DUSE_AVX=ON -DUSE_SSE4_1=ON -DUSE_SSE4_2=ON
        "$CMAKE_BIN" --build "$build_dir" -j"$(nproc 2>/dev/null || echo 2)" --target Jolt
    fi
    if [ -f "$build_dir/libJolt.a" ]; then
        cp -f "$build_dir/libJolt.a" "$out/libJolt.a"
    elif [ -f "$build_dir/Release/libJolt.a" ]; then
        cp -f "$build_dir/Release/libJolt.a" "$out/libJolt.a"
    else
        # search
        found=$(find "$build_dir" -name 'libJolt.a' 2>/dev/null | head -1)
        if [ -n "$found" ]; then
            cp -f "$found" "$out/libJolt.a"
        else
            echo "libJolt.a not found after build" >&2
            return 1
        fi
    fi
    return 0
}

build_real_shim() {
    echo "building real ori_jolt_shim against Jolt…"
    tmp=$(mktemp -d)
    # Match libJolt portable flags (AVX+SSE4, no AVX2).
    jolt_flags="-O3 -DNDEBUG -fPIC -std=c++17 -fno-rtti -fno-exceptions -ffp-contract=off -mavx -msse4.1 -msse4.2 -mfpmath=sse -pthread -DJPH_DEBUG_RENDERER -DJPH_OBJECT_STREAM -DJPH_PROFILE_ENABLED -DJPH_USE_AVX -DJPH_USE_SSE4_1 -DJPH_USE_SSE4_2"
    # shellcheck disable=SC2086
    g++ -c $jolt_flags \
        -I"$root/vendor/JoltPhysics" \
        -o "$tmp/shim.o" \
        "$root/native/ori_jolt_shim.cpp" || {
        rm -rf "$tmp"
        return 1
    }
    # Separate archives (link order: ori_jolt_shim then Jolt via pkg.toml).
    rm -f "$out/libori_jolt_shim.a"
    ar rcs "$out/libori_jolt_shim.a" "$tmp/shim.o"
    rm -rf "$tmp"
    echo "REAL=1" > "$out/ORI_JOLT_MODE.txt"
    echo "real Jolt shim → $out/libori_jolt_shim.a (+ libJolt.a)"
}

if [ "$force_stub" -eq 1 ]; then
    build_stub
    exit 0
fi

if build_jolt_lib && build_real_shim; then
    exit 0
fi

echo "falling back to Euler stub"
build_stub
