# Changelog

## [Unreleased]

### Added
- **`box2d.world_sync`:** bridge `game.world` entities ↔ Box2D bodies (`PhysicsHost.body_handle`); smoke `tools/smoke_world_sync.sh`.
- Integration demo under `demos/` (with ori-game path dep).
- **P2-D `box2d.debug_draw`:** View2D + box/circle/joint/contact/ray helpers via
  `game.draw`; `demos/box2d_visual` uses the module.

### Changed
- **Module rename:** `box2d.world` → **`box2d.dynamics`** (file `box2d/dynamics.orl`) — Ori import search clashes with `game.world`.

## [0.3.0] - 2026-07-14

### Added (Wave 3 — jam-ready product surface)
- Angular velocity get/set: `body_omega_mdeg` / `set_angular_velocity` (milli-deg/s)
- Materials: `set_friction` / `set_restitution` (+ getters) on all body shapes (coeff ×1000)
- Convex `create_static_poly4` / `create_dynamic_poly4` (8 local milli-int verts)
- Joints: distance, revolute, weld — create/destroy/`joint_is_valid` (int slots)
- Queries: `raycast` + last hit x/y; AABB query count + body buffer
- Contacts: after `step`, `contact_count` + `contact_body_a/b(i)`
- Example `joints_demo.orl`; smoke covers joints_demo
- Units helpers: degrees/mdeg, angular velocity, material milli

## [0.2.0] - 2026-07-13

### Added
- circles, kinematic boxes, velocity/force/impulse, units module, impulse_test demo

## [Unreleased]

### Added
- Integration demo `demos/box2d_visual` (ori-game + Box2D).
