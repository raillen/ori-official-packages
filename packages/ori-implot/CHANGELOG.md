# Changelog

## [0.2.0] - 2026-07-15

### Added
- Maturity **5 (Linux)**: line + scatter + bar draw surface; axis labels; clear/reset
- Dual build path: headless `libori_implot_shim` (tests/smoke) + `libori_implot_full.a` (FULL ImPlot)
- `setup_axes`, label queries, `last_draw`, `capacity`, `is_full`
- ≥5 automated tests (stats, clear/reset, axes, draw kinds, capacity edge)

### Changed
- Version `0.1.0` → `0.2.0`
- Smoke requires FULL artifact when sibling `ori-imgui` is available

## [0.1.0] - 2026-07-15

### Added
- ImPlot vendor + shim (line/bars)
- Series milli buffer + headless stats tests
