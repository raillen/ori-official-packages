# Changelog

## [0.2.0] - 2026-07-15

### Added
- Triangle-soup builder: `mesh_clear`, `mesh_add_vertex`, `mesh_add_triangle`, counts, `build_mesh`
- Optional agent radius on `build_mesh` / `build_plane_ex` (milli; `<=0` → default 0.3 m)
- `find_nearest` + `nearest_x/y/z` (Detour nearest poly)
- `raycast` + `raycast_t_m` / `raycast_hit_*` (navmesh surface raycast)
- Tests: soup path, nearest, raycast, agent radius / empty soup edges
- Smoke covers soup build, path, nearest, raycast, destroy

### Changed
- Version **0.1.0 → 0.2.0** (maturity 5 Linux product surface)
- README API table expanded

## [0.1.0] - 2026-07-15

### Added
- `recast.nav` module (Recast + Detour navmesh MVP)
- Shim: `build_plane`, `find_path`, `waypoint_*`, `is_ready`, `destroy` (milli-units)
- Vendor Recast Navigation (`vendor/recastnavigation`)
- Linux `tools/build_linux.sh` + `tools/smoke_linux.sh`
- Example `examples/smoke_recast.orl` and unit tests
