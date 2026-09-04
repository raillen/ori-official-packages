# ori-jolt

[Jolt Physics](https://github.com/jrouwe/JoltPhysics) bindings for
[Ori](https://github.com/raillen/ori-lang) (**S3 / 0.3.x**) — ECO package **P3**.

**Package:** `jolt` · **Version:** `0.2.0`  
**Modules:** `jolt.world`, `jolt.units`, `jolt.character_ctrl`, `jolt.vehicle_arcade`  
**Requires:** Ori ≥ 0.3.0 · system linker · C++17 (real Jolt) or C stub (CI)

## Units

Same boundary strategy as `ori-box2d` (float→C packaging safety):

| Quantity | Encoding |
|----------|----------|
| Position / size / velocity / impulse | meters × 1000 |
| Angular velocity / torque | rad/s or N·m × 1000 |
| Density / friction / restitution | × 1000 (1.0 → 1000) |
| `step` dt | microseconds |
| Quaternion components | × 1000 |
| Object layer mask | bit 0…3 for layers 0…3 |

Helpers: `jolt.units.meters_to_milli`, `seconds_to_micros`, `gravity_earth_y_m`, …

## API

| Area | Functions |
|------|-----------|
| System | `create_system`, `destroy_system`, `set_gravity`, `step` |
| Bodies | static/dynamic **box**, static/dynamic **sphere**, dynamic **capsule**, **`create_static_floor(y)`** |
| Motion | get position/velocity, `set_position`, `set_linear_velocity`, `add_impulse`, **`add_torque`**, **`body_w*_m`** |
| Materials | **`set_friction` / `set_restitution`** (milli), getters |
| Layers | **`set_body_layer`**, **`set_layer_mask`** (4 layers: static/dynamic/player/debris) |
| Query | `raycast` + `last_hit_*_m` + **`last_hit_body`** |

## Build

```bash
# Fast headless (Euler stub, same ABI — product smoke)
./tools/build_linux.sh --stub

# Real Jolt (vendored vendor/JoltPhysics, needs cmake + g++)
./tools/build_linux.sh           # tries real (portable AVX+SSE4), falls back to stub
# or force:
#   ./tools/build_linux.sh --stub
```

Artifacts: `lib/<triple>/libori_jolt_shim.a` + `libJolt.a`.

**Real Jolt link:** use a C++ driver so libstdc++ resolves, e.g.

```bash
ORI_USE_SYSTEM_LINKER=1 ORI_NATIVE_LINKER=g++ ori compile examples/boxes_fall.orl --out boxes
```

```bash
./tools/smoke_linux.sh           # stub by default (fast CI)
ORI_JOLT_REAL=1 ./tools/smoke_linux.sh   # real Jolt (needs g++ link)
```

## Constraints

| API | Role |
|-----|------|
| `create_fixed_constraint` | weld two bodies |
| `create_distance_constraint` | min/max rope (milli-m) |
| `create_hinge_constraint` | hinge axis (milli-unit direction) |
| `destroy_constraint` | remove |

## Character controller

Jolt **CharacterVirtual** (real) / Euler capsule (stub):

| API | Role |
|-----|------|
| `create_character(sys, x,y,z, radius, half_height)` | milli-m |
| `character_set_velocity` / `character_jump` | movement |
| `character_on_ground` / `character_*_m` | query |
| `destroy_character` | cleanup |

Updated each `step`. Frame helper: `jolt.character_ctrl`.

## Vehicle (constraints MVP)

Chassis dynamic box + 4 hinged wheels (real) + drive forces on chassis:

| API | Role |
|-----|------|
| `create_vehicle(sys, x,y,z, half_extents)` | milli-m |
| `vehicle_set_input(throttle, steer, brake)` | −1000…1000 milli |
| `vehicle_chassis` | body handle for pose/draw |
| `destroy_vehicle` | cleanup |

Arcade modes: `jolt.vehicle_arcade`.

## Link with raylib 3D

**Ready-made visual demo:**

```bash
cd demos/jolt_boxes_3d
./tools/smoke.sh
# or: ./tools/stage_libs.sh && ORI_NATIVE_LINKER=g++ ori compile main.orl --out boxes3d
```

That demo package depends on `jolt` + `ori_game` and stages all four
static libs (`ori_jolt_shim`, `Jolt`, `ori_raylib_shim`, `raylib`) into its
own `lib/<triple>/` (driver only links `native_libs` of the entry package).

## Docs

- `docs/p3-jolt-spike.md` — spike / packaging notes  
- Plan: ori-lang `docs/planning/eco-game-imgui-raylib3d-plan.md` §8

## License

MIT (this package). Jolt Physics is MIT — see `vendor/JoltPhysics/LICENSE`.
