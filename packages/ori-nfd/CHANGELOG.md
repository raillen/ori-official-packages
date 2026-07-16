# Changelog

## [0.2.0] - 2026-07-15

### Added
- Multi-select open: `open_files` + `path_count` / `path_at_len` / `path_at_byte`
- Path marshalling: `path_byte`, `path_sum`, `path_ptr` (headless-safe, no string-from-ptr)
- `clear()` and empty inject → cancel / empty state
- `set_paths_for_test` (semicolon-separated) for multi path tests
- Env `NFD_SMOKE_SKIP_UI=1` skips real dialogs (returns cancel; default in smoke)
- ≥5 unit tests (set path, bytes, clear/cancel, multi, empty cancel)
- Maturity **5 (Linux)** claim — G1..G7

### Changed
- Version **0.2.0**; README API table + Phase OS deferred note

## [0.1.0] - 2026-07-15

### Added
- portable-file-dialogs shim + `nfd.dialog`
- Test hook `set_path_for_test` for headless CI
