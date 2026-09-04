# P3 spike — Jolt packaging (2026-07-13)

## Choice

| Item | Decision |
|------|----------|
| Engine | **Jolt Physics v5.2.0** (vendored) |
| C boundary | **Custom `ori_jolt_*`** shim (not joltc/JoltC) — same control as ori-box2d |
| Units | **milli-int** (meters×1000, µs dt) |
| Fallback | `native/ori_jolt_stub.c` Euler + floor (CI / no cmake) |

## Why custom shim vs joltc

1. Single static archive for Ori `native_libs`.
2. Explicit milli-unit contract (Ori float FFI issues in packages).
3. Minimal surface: system, box/sphere/capsule, step, raycast.
4. joltc/JoltC add C# / extra build graphs; spike kept lean.

## Build outcomes

| Mode | Command | Result |
|------|---------|--------|
| Stub | `./tools/build_linux.sh --stub` | `libori_jolt_shim.a` + dummy `libJolt.a` |
| Real | cmake + g++ against `vendor/JoltPhysics` | `libori_jolt_shim.a` + real `libJolt.a` |

**SIMD:** default cmake uses **AVX + SSE4** (no AVX2) for wider Linux CPUs.  
**Link:** real mode needs `ORI_NATIVE_LINKER=g++` (or equivalent) for libstdc++.

## Smoke acceptance

- `examples/boxes_fall.orl` prints `fell`
- `examples/impulse_test.orl` prints `moved`

## Residual

- Character controller / constraints / multi-thread tuning — later slices  
- Visual demo with ori-game 3D — optional `examples/jolt_visual` when path deps supported in driver
