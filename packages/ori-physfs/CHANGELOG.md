# Changelog

## [0.2.0] - 2026-07-15

### Added
- Write path: `set_write_dir`, `mkdir`, `write` / `write_bytes`, `last_write_len`
- Multi-mount product surface (stack mounts + read across mount points)
- Unmount + multi-mount automated tests (temp dirs)
- Smoke covers multi-mount, write-dir write/read-back, unmount
- Maturity **5 (Linux)** gate (G1–G7)

### Changed
- Package version `0.1.0` → `0.2.0`
- README API table expanded for write + multi-mount

## [0.1.0] - 2026-07-15

### Added
- `physfs.fs` bindings: init/deinit, mount/unmount, exists, read + last_* getters, enumerate_count
- Linux build (system libphysfs or vendored PhysFS 3.2.0)
- Smoke example + unit tests over temp mount
