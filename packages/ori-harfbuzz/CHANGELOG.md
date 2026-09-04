# Changelog

## [0.1.0] - 2026-07-15

### Added (complete surface → maturity 5)
- Direction shaping + `harfbuzz.layout` (shape → FreeType placed glyphs)
- AOT `ori test` via staged libharfbuzz (static or GNU ld-script → .so)
- RTL test + cursor positions; path-dep FreeType native_libs

### Added
- HarfBuzz shim + `harfbuzz.shape` (UTF-8 shape on FreeType faces)
- Linux build/smoke (`tools/build_linux.sh`, `tools/smoke_linux.sh`)
