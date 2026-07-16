# Changelog

## [Unreleased]

### Notes
- **P2-C glue** lives in **`ori-game`** (`game.rres_assets`): ORPK extract →
  `AssetCache`. Consumers path-dep both packages; see `ori-game/demos/rres_assets`.

## [0.3.0] - 2026-07-14

### Added
- `validate` / `validate_ok` — structural ORPK check without loading
- `open` validates magic/version/entry bounds before load
- `is_valid` on open handles
- `list_names` — all entry names (newline-separated)
- `read_bytes_to_path` — explicit binary extract (alias of export)
- `last_error` for validate/open failures
- README documents full ORPK layout

## [0.2.0] - 2026-07-13

### Added
- has/put/remove/name_at/read_text, pack_cli
