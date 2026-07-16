# ori-noise

[FastNoiseLite](https://github.com/Auburn/FastNoiseLite) procedural noise for Ori (S3).

**Package:** `noise` · **Module:** `noise.fnl` · **Version:** 0.2.0  
**Maturity:** **5 (Linux)** product surface (API + tests + smoke)

## Milli convention

Coordinates, samples, frequency, lacunarity, and gain use **milli-int** units
(`1000` = `1.0`). Noise samples are typically in roughly `[-1000, 1000]`
(FNL returns `[-1, 1]`).

## API (`noise.fnl`)

| Function | Role |
|----------|------|
| `init()` | Reset global state to FastNoiseLite defaults |
| `set_seed(seed)` | Deterministic seed |
| `set_frequency_m(freq_m)` | Base frequency (milli) |
| `use_open_simplex()` | OpenSimplex2 |
| `use_perlin()` | Perlin |
| `use_cellular()` | Cellular / Worley |
| `use_value()` | Value noise |
| `use_fractal_none()` | Single-octave (no fractal) |
| `use_fractal_fbm()` | Fractal Brownian Motion |
| `use_fractal_ridged()` | Ridged multi-fractal |
| `set_octaves(n)` | Fractal octave count (clamped 1–32) |
| `set_lacunarity_m(m)` | Frequency multiplier per octave (milli) |
| `set_gain_m(m)` | Amplitude multiplier per octave (milli) |
| `get2_m(x, y)` | Sample 2D (milli in / milli out) |
| `get3_m(x, y, z)` | Sample 3D (milli in / milli out) |

State is a **process-global** singleton (one concurrent generator). Call `init()`
before configuring a new field.

## Build / smoke (Linux)

```bash
export ORI_BIN=/path/to/ori
export ORI_RUNTIME_LIB=/path/to/libori_runtime.a
export ORI_RUNTIME_CDYLIB=/path/to/libori_runtime.so
export ORI_USE_SYSTEM_LINKER=1 ORI_USE_JIT=1

./tools/build_linux.sh
./tools/smoke_linux.sh
```

Smoke prints `ok` and runs `ori test` (≥4 cases).

## Path dep

```toml
[package]
name = "my_game"
native_libs = ["ori_noise_shim"]

[dependencies]
noise = { path = "../ori-noise", version = "0.2.0" }
```

Link `lib/x86_64-unknown-linux-gnu/libori_noise_shim.{a,so}` (or set
`LD_LIBRARY_PATH` for JIT).

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

Shim and Ori surface: MIT. FastNoiseLite: MIT (`vendor/FastNoiseLite.h`).
