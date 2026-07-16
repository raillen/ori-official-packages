# Changelog

## [0.2.0] - 2026-07-15

### Added
- Fractal modes: `use_fractal_none`, `use_fractal_fbm`, `use_fractal_ridged`
- `set_octaves`, `set_lacunarity_m`, `set_gain_m` (milli)
- Noise type `use_value` (Value noise) alongside open simplex / perlin / cellular
- Expanded automated tests (≥4): types differ, seed changes field, FBM vs none, cellular 3D range
- Smoke exercises 2D + fractal 3D path
- README API table + Phase OS note; maturity **5 (Linux)**

### Changed
- Package version `0.1.0` → `0.2.0`

## [0.1.0] - 2026-07-15

### Added
- FastNoiseLite C shim + `noise.fnl` (seed, frequency, 2D/3D milli)
- Linux smoke + tests
