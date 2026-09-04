# Changelog

## [0.2.0] - 2026-07-15

### Added
- `stb.perlin.noise2_m` (2D Perlin; z=0 sample)
- `stb.image.write_solid_gray_png` for easy write/load roundtrips
- `stb.rect_pack.rect_w` / `rect_h` / `rect_count`
- Broader tests: perlin 2D/3D, rect pack success + tight fail, image missing + PNG roundtrip
- Smoke exercises image write/load + perlin2/3 + pack get

### Changed
- Version bump for maturity **5 (Linux)** (G1–G7)

## [0.1.0] - 2026-07-15

### Added
- `stb.image` (stb_image + write PNG gray)
- `stb.perlin`, `stb.rect_pack`
- Linux build/smoke + unit tests
