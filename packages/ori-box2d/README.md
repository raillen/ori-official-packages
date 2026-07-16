# ori-box2d

[Box2D](https://box2d.org/) **3.1** for Ori (S3) — jam-ready milli-unit product surface (Linux-5 / Wave 3).

**Package:** `box2d` · **Modules:** `box2d.dynamics`, `box2d.units`, `box2d.world_sync` · **Version:** `0.3.0`  
> **Rename:** `box2d.world` → `box2d.dynamics` (avoids Ori import clash with `game.world`).

## Units

FFI boundary uses **integers** (Ori float→C was unreliable in packaging):

| Quantity | Encoding |
|----------|----------|
| Length / position / velocity / force / impulse | meters × 1000 |
| Density | × 1000 |
| `step` dt | microseconds |
| Angle | millidegrees (deg × 1000) |
| Angular velocity | **milli-deg/s** (deg/s × 1000) |
| Friction / restitution | coeff × 1000 (0.5 → `500`) |

Helpers: `box2d.units.meters_to_milli`, `seconds_to_micros`, `degrees_to_mdeg`, `deg_per_s_to_mdeg`, `material_to_milli`, `gravity_earth_y_m`, …

## World sync (`box2d.world_sync`)

Bridge **game.world** entities ↔ Box2D bodies (`PhysicsHost.body_handle`).  
Smoke: `tools/smoke_world_sync.sh` (crate falls, transform updates).

## API table (`box2d.dynamics`)

| Area | Functions |
|------|-----------|
| **World** | `create_world`, `destroy_world`, `is_valid`, `set_gravity`, `step` / `step_substeps` |
| **Bodies** | `create_static/dynamic/kinematic_box`, `create_static/dynamic_circle`, `create_static/dynamic_poly4`, `destroy_body`, `body_is_valid`, `live_body_count` |
| **Pose & motion** | `body_x/y_m`, `body_angle_mdeg`, `body_vx/vy_m`, `body_omega_mdeg`, `set_position`, `set_linear_velocity`, `set_angular_velocity`, `apply_force_to_center`, `apply_impulse_to_center`, `set_awake` |
| **Materials** | `set_friction`, `set_restitution`, `body_friction_m`, `body_restitution_m` (all shapes on body) |
| **Joints** | `create_distance_joint`, `create_revolute_joint`, `create_weld_joint`, `destroy_joint`, `joint_is_valid` (`Joint` handle = int slot; anchors = world milli-m; distance `length_m <= 0` → current span) |
| **Queries** | `raycast` + `last_ray_hit` / `last_ray_x_m` / `last_ray_y_m`; `query_aabb` + `aabb_result_count` / `aabb_result_body` |
| **Contacts** | after each `step`: `contact_count`, `contact_body_a(i)`, `contact_body_b(i)` |

### Poly4

`create_dynamic_poly4(w, x, y, x0,y0, x1,y1, x2,y2, x3,y3, density_m)` — body at `(x,y)`; four **local** verts in milli-meters (convex hull). Same for static without density.

## Build / smoke

```bash
./tools/build_linux.sh
./tools/smoke_linux.sh   # boxes_fall + impulse_test + joints_demo
```

Env: `ORI_BIN` (default `ori`), `ORI_USE_SYSTEM_LINKER=1` recommended for packaged Ori without rustc link driver.
