# Changelog

## [0.2.0] - 2026-07-15

### Added
- Buffer compress/decompress (`compress` / `decompress` + out accessors)
- CRC32 buffer API (`crc32` / `crc32_text`)
- In-memory ZIP **create one entry** and **extract one entry** (`zip_create_entry`, `zip_extract_last`, `zip_extract`, `zip_roundtrip_len`)
- Build includes `miniz_zip.c` with archive APIs enabled (stdio/time still disabled)
- ≥4 automated tests (round-trip, CRC, ZIP, decompress error)
- README API table + Phase OS note

### Changed
- Version **0.2.0** — maturity **5 (Linux)** (G1–G7)

## [0.1.0] - 2026-07-15

### Added
- miniz compress/uncompress + CRC32 text helpers
- Linux smoke + tests
