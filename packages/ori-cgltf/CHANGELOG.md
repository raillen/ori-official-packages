# Changelog

## [0.2.0] - 2026-07-15

### Added (complete surface → maturity 5 Linux)

- Mesh/primitive queries: `mesh_primitive_count`, `mesh_vertex_count`, `mesh_index_count`, `mesh_has_position`, `mesh_has_normal`
- Node TRS milli: `node_has_*`, `node_translation_m`, `node_rotation_m`, `node_scale_m`
- Material `material_base_color_m` (PBR base color RGBA milli)
- **Export interleaved float mesh** for raylib upload helpers: `export_mesh` layout `[px,py,pz,nx,ny,nz]*V` + index buffer, milli/index accessors, native `export_*_ptr`
- Fixture: POSITION+NORMAL triangle with node TRS and tinted base color
- Tests: 5 cases (counts, missing, TRS/color, export, reload)
- Version **0.2.0** — maturity **5 (Linux)** (G1–G7)

## [0.1.0] - 2026-07-15

### Added
- `cgltf.loader` module (cgltf single-header glTF 2.0 parser)
- Shim: `load_file`, `free`, `mesh_count`, `node_count`, `material_count`, `animation_count`, `mesh0_vertex_count`
- Linux build/smoke + unit tests with `tests/fixtures/minimal.gltf`
