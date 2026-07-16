# Changelog

## [Unreleased]

### Added
- Integration demo under `demos/` (with ori-game path dep).

## [0.3.0] - 2026-07-14

### Added
- Prepared statements: `prepare`, `bind_int`, `bind_text`, `step`,
  `column_int`, `column_text` / `column_text_to_path`, `column_count`,
  `reset`, `clear_bindings`, `finalize`
- `STEP_ROW` / `STEP_DONE` constants
- `query_all_to_json_path` / `query_all_json` multi-row helper
- `examples/prepared_demo.orl`

## [0.2.0] - 2026-07-13

### Added
- transactions, query_text, changes, last_error, tx_rollback demo

## [Unreleased]

### Added
- Integration demo `demos/score_game` (sqlite + ori-game).
