# Changelog

## [0.1.0] - 2026-07-15

### Added
- Maturity **5 (Linux)**: 3D line + scatter series (milli); axis labels XYZ; clear/reset
- Dual build path: headless `libori_implot3d_shim` (tests/smoke) + `libori_implot3d_full.a` (FULL ImPlot3D)
- Vendor [brenocq/implot3d](https://github.com/brenocq/implot3d) (core + items; no demo/meshes for G1)
- `setup_axes`, label queries, `last_draw`, `capacity`, `is_full`
- ≥5 automated tests (stats, clear/reset, axes, draw kinds, capacity edge)
- Smoke (`examples/smoke_implot3d.orl`) + `tools/smoke_linux.sh`
- Path-dep documented: sibling `ori-imgui` + `ori-implot` ecosystem
