# Changelog

## [0.1.0] - 2026-07-15

### Added (complete surface → maturity 5)
- Full FreeType face API + **`freetype.atlas`** (shelf pack, gray8 UV metrics, PGM)
- Kerning, bitmap sample, `freetype.text` metrics
- Build stages static + shared FreeType chain for AOT/`ori test` and JIT
- Smoke + **5 unit tests** (incl. `atlas_ascii_pack`)

### Added
- FreeType shim + `freetype.face` Ori module (init, face, pixel sizes, render char, metrics)
- `face_ptr` export for HarfBuzz (`hb_ft_font_create_referenced`)
- Linux build (`tools/build_linux.sh`) against system `freetype2`
- Smoke tests (`tools/smoke_linux.sh`, `tests/test_freetype.orl`)
