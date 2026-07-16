# Changelog

## [Unreleased]

### Added
- **R4.3 `jolt.world_character`:** CharacterVirtual + static solids from World/level
  (game units → milli); `examples/character_world_test.orl` → `world_char_ok`.
- **P2-D `jolt.debug_draw`:** box/sphere/capsule/constraint/grid helpers via
  `game.draw3d`; `demos/jolt_boxes_3d` uses the module. Path-dep `ori_game`.

### Changed
- **Module rename:** `jolt.world` → **`jolt.dynamics`** (file `jolt/dynamics.orl`) — Ori clash with `game.world`.

## [0.2.0] - 2026-07-14

### Added (Wave 4 — Linux maturity 5)

- **Raycast hit body:** `last_hit_body` / `last_hit_body_id` after successful `raycast`
- **Friction / restitution (milli):** `set_friction`, `set_restitution`, getters
- **Object layers (0–3):** `set_body_layer` / `get_body_layer`,
  `set_layer_mask` / `get_layer_mask` (bitmask of collidable layers),
  helpers `layer_static` / `layer_dynamic` / `layer_player` / `layer_debris`
- **Angular motion:** `body_wx_m` / `body_wy_m` / `body_wz_m`, `add_torque`
- **Static floor helper:** `create_static_floor(sys, y_m)` — top surface at `y_m`
- Example `examples/wave4_test.orl` → `wave4_ok`
- Character / boxes_fall examples use `create_static_floor`

### Notes

- Stub (`ori_jolt_stub.c`) remains default smoke path; ABI-compatible no-ops /
  simple Euler for new symbols
- Real Jolt optional via `./tools/build_linux.sh` (no `--stub`)

## [0.1.0] - 2026-07-13

### Added (residual — 2026-07-13)
- Constraints: `create_fixed_constraint`, `create_distance_constraint`,
  `create_hinge_constraint`, `destroy_constraint`
- Example `examples/constraint_test.orl`
- Visual demo package `demos/jolt_boxes_3d` (Jolt + `ori-game` raylib 3D),
  `tools/stage_libs.sh` + `tools/smoke.sh`
- **CharacterVirtual:** `create_character`, set velocity/jump, ground query,
  position getters — example `character_test.orl` (`walked`)
- **Vehicle (chassis + hinge wheels + drive forces):** `create_vehicle`,
  `vehicle_set_input`, `vehicle_chassis` — example `vehicle_test.orl` (`drove`)
- **Character frame API:** `jolt.character_ctrl` (`set_wish_dir`, `tick`) —
  `character_frame_test` → `framed`
- **Arcade vehicles L1:** `jolt.vehicle_arcade` land / plane / boat —
  `vehicle_modes_test` → `modes_ok`

### Added (trilha P3 — product MVP)

- Package `jolt` for Ori S3: `jolt.world` + `jolt.units`
- C API `ori_jolt_*`: system, static/dynamic box & sphere, dynamic capsule,
  pose/velocity/impulse, raycast, live body count
- Real shim: `native/ori_jolt_shim.cpp` over vendored **Jolt Physics 5.2**
- Headless stub: `native/ori_jolt_stub.c` (Euler + floor) for CI
- Examples: `boxes_fall`, `impulse_test`
- Tools: `build_linux.sh`, `smoke_linux.sh`
- Docs: README, `docs/p3-jolt-spike.md`
