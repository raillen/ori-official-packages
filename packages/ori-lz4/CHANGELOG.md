# Changelog

## [0.2.0] - 2026-07-15

### Added
- OLZ1 stream/frame compress/decompress (`stream_compress`, `stream_compress_text`, `stream_decompress`, `stream_roundtrip_len`) using LZ4 stream continue
- Bound/size helpers: `compress_bound`, `stream_bound`
- Larger pattern fixtures: `pattern_roundtrip`, `pattern_stream_roundtrip` (native fill + verify)
- Buffer compress path: `compress` / `decompress`
- Out accessors: `out_sum`, `last_src_len`
- Tests: stream round-trip, compress_bound, larger fixture (32 KiB), garbage decompress fails
- Smoke covers stream + bound + 8 KiB pattern

### Changed
- Maturity **5 (Linux)** — G1–G7
- Version **0.2.0**

## [0.1.0] - 2026-07-15

### Added
- LZ4 block compress/decompress (`compress_text`, `roundtrip_len`)
- Prefer system liblz4 or vendored `lz4.c`/`lz4.h` (v1.10.0)
- Linux smoke + tests (round-trip length; repetitive text smaller)
