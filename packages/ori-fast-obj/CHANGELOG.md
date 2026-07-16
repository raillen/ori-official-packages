# Changelog

## [0.2.0] - 2026-07-15

### Added
- Maturity **5 (Linux)** product surface (G1–G7)
- Attribute samples: `normal_m`, `texcoord_m` (milli)
- Face-corner indices: `index_p` / `index_t` / `index_n`
- Materials: `material_count`, `material_name_len`, `material_name_byte`
- Flatten helper `export_mesh`: interleaved `[px,py,pz,nx,ny,nz,u,v]*V` + indices
- Export queries: vertex/index/float counts, milli accessors, raw pointers, `export_sum_m`
- Fixture `triangle.mtl` + texcoords on `triangle.obj`
- Tests: normals/uvs/indices, export fingerprint, reload clears export (5 total)

### Changed
- Version `0.1.0` → `0.2.0`
- README API table + flatten layout documentation

## [0.1.0] - 2026-07-15

### Added
- `fast_obj.mesh` (Wavefront OBJ via fast_obj single-header)
- Shim: load / free / position_count / face_count / normal_count / texcoord_count / index_count / pos_m
- Fixture `tests/fixtures/triangle.obj`
- Linux build/smoke + unit tests
