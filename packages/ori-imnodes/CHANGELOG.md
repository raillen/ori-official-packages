# Changelog

## [0.2.0] - 2026-07-15

### Added
- Product editor API: `begin/end_node_editor`, `begin/end_node`, input/output attrs, `link`
- Pin bookkeeping: `graph_add_pin`, `graph_pin_count`, `graph_pin_id` / `graph_pin_kind`
- Link queries: `graph_add_link_id`, `graph_link_id` / `graph_link_start` / `graph_link_end`
- `graph_has_node`; pin kind constants `PIN_INPUT` / `PIN_OUTPUT`
- ≥5 automated bookkeeping tests (no human clicks); capacity edge case
- README API table + FULL vs headless modes
- Maturity **5 (Linux)** — G1–G7

### Changed
- Package version `0.1.0` → `0.2.0`

## [0.1.0] - 2026-07-15

### Added
- imnodes vendor + shim + graph bookkeeping for tests
