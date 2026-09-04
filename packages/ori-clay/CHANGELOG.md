# Changelog

## [0.2.0] - 2026-07-15

### Added
- Nested multi-box layout (`layout_nested`): Root TTB + Body LTR with padding/gap
- Named box indices: `BOX_HEADER`, `BOX_BODY`, `BOX_LEFT`, `BOX_RIGHT`
- Command list export for drawers: `cmd_count`, `cmd_type`, `cmd_x/y/w/h_m`, `cmd_r/g/b/a`, `rect_cmd_count`
- Command type constants: `CMD_RECTANGLE` (and NONE/BORDER/TEXT)
- Headless tests for pad/gap, nested bounds, command rects, no-init edge (≥4)

### Changed
- `layout_two_boxes` also fills the command list after end
- Maturity **5 (Linux)** — G1..G7

## [0.1.0] - 2026-07-15

### Added
- Vendor Clay (`clay.h`) single-header layout library
- C shim `ori_clay_shim`: init, headless two-box layout, milli bounds
- Module `clay.layout`
- Linux build/smoke + unit tests
