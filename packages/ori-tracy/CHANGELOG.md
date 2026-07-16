# Changelog

## [0.2.0] - 2026-07-15

### Added
- Message/plot headless counters (`message_count`, `plot_count`, `last_plot_value_m`)
- Edge counters (`overflow_begins`, `orphan_ends`)
- Dual build: headless `libori_tracy_shim` + FULL `libori_tracy_full.a` (TracyClient)
- Proper FULL zone begin/end via TracyC (`___tracy_emit_zone_begin_alloc` / end)
- ≥5 automated tests (zones, frames, message/plot, orphan end, reset/flag)
- Smoke verifies FULL artifact symbols without requiring Tracy GUI

### Changed
- Version **0.1.0** → **0.2.0** (maturity 5 Linux)
- README API table + Phase OS note

## [0.1.0] - 2026-07-15

### Added
- Zone/frame instrumentation API + headless stats
- Optional `ORI_TRACY_FULL=1` TracyClient build
- Linux smoke + unit tests
