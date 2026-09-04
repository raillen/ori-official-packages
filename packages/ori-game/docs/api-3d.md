# API 3D — `game.camera3d` / `game.draw3d`

Convention: **snake_case** functions, **PascalCase** types, **`public`** visibility.

## `game.camera3d`

```ori
import game.camera3d = cam3
import ori.math.vec3 = vec3

var cam: cam3.Camera3D = cam3.create_perspective(
    vec3.Vec3 { x: 4.0, y: 3.0, z: 4.0 },
    vec3.Vec3 { x: 0.0, y: 0.0, z: 0.0 },
    45.0
)

cam3.begin_mode(cam)
-- draw3d ...
cam3.end_mode()
```

| API | Notes |
|-----|--------|
| `Camera3D` | position, target, up, fovy, projection |
| `PROJECTION_PERSPECTIVE` / `ORTHOGRAPHIC` | int constants |
| `create` / `create_perspective` | factories |
| `begin_mode` / `end_mode` | wraps `ori_rl_Begin/EndMode3D` |
| `position_of` / `target_of` / `up_of` / `fovy_of` / `projection_of` | getters |
| `set_position` / `look_at` / `set_fovy` | return updated camera |

## `game.draw3d`

```ori
import game.draw3d = d3
import game.color = color

d3.draw_grid(10, 1.0)
d3.draw_cube(pos, 1.0, 1.0, 1.0, color.RED)
const model: d3.ModelId = d3.load_model("assets/cube.obj")
d3.draw_model(model, pos, 1.0, color.WHITE)
d3.unload_model(model)
```

| API | L0 |
|-----|-----|
| `draw_cube` / `draw_cube_wires` | `ori_rl_DrawCube*` |
| `draw_sphere` / `draw_sphere_wires` | `ori_rl_DrawSphere*` |
| `draw_plane` | `ori_rl_DrawPlane` |
| `draw_grid` | `ori_rl_DrawGrid` |
| `draw_ray` | `ori_rl_DrawRay` |
| `load_model` / `unload_model` | handle table in shim |
| `draw_model` / `draw_model_ex` | position + scale / axis-angle |

Model format for demos: **`.obj`** (lightest for MVP). glTF later.

**Raylib 5.5 caveat:** `LoadModel` on `.obj` **requires vertex normals** (`vn` / `f v//vn` or `f v/vt/vn`). Exports with only `f v/vt` (common Blender/Tripo) **segfault** in raylib’s tinyobj path. Fix assets with:

```bash
python3 tools/obj_inject_normals.py assets/foo.obj --in-place
```

## Handles

See `game.ids`: `ModelId`, `TextureId`, `SoundId`, `MusicId`, `FontId` (`public alias` → `int`).

## `game.ray3d` (R3)

| API | Role |
|-----|------|
| `mouse_ray(mouse, cam) -> Ray3` | screen pick ray for camera |
| `hit_sphere` / `hit_box` / `hit_ground` | collision queries |
| `last_hit() -> Hit3` | point, normal, distance after a hit |
| `draw_bounding_box` | debug AABB |

## Shaders (`game.shader`)

| API | Role |
|-----|------|
| `load` / `load_fragment` / `unload` | LoadShader |
| `begin_mode` / `end_mode` | Begin/EndShaderMode |
| `location` / `set_float` / `set_vec3` / `set_vec4` / `set_int` | uniforms |

Sample GLSL: `assets/shaders/lighting.vs` + `lighting.fs`.

## Lights (`game.light`)

| API | Role |
|-----|------|
| `create_directional` / `create_point` | Light struct |
| `upload(shader, light, locs…)` | write uniforms (not named `apply` — keyword) |

## Model texture / animation (R3 residual)

| API | Role |
|-----|------|
| `set_model_diffuse_texture(model, texture)` | MATERIAL_MAP_DIFFUSE |
| `load_model_animations` / `unload_model_animations` | anim set handle |
| `model_animation_count` | clips in set |
| `update_model_animation(model, set, index, frame)` | skin pose |
| `update_model_animation_in_place(...)` | CPU skin, root XZ zeroed |
| `update_model_animation_bones(...)` | **GPU** skin: fill `mesh.boneMatrices` only |
| `update_model_animation_bones_in_place(...)` | GPU + root XZ zeroed |
| `model_supports_gpu_skinning` / `model_set_all_materials_shader` | enable skinning shader |

GPU path needs `assets/shaders/skinning.vs` + `skinning.fs` (raylib `models_gpu_skinning` style) assigned to the model materials before drawing.

### `game.anim_player`

| API | Role |
|-----|------|
| `create` / `play` / `stop` / `tick(dt)` | clip player |
| `set_clip` / `set_in_place` / `toggle_in_place` | clip + root lock |
| `draw_posed` | draw with yaw/position pose |
| Procedural clips | `CLIP_IDLE`, `CLIP_WALK`, `CLIP_SPIN`, `CLIP_BOW` (static meshes) |

## Examples

- `examples/hello_3d.orl` — grid + cube + sphere + camera nudge + optional `.obj`
- `examples/cangaceiro_demo.orl` — real low-poly character OBJ + diffuse texture + orbit camera
- `examples/pick_3d.orl` — mouse ray vs sphere + bounding box
- Sibling `ori-jolt/demos/jolt_boxes_3d` — Jolt dynamics + this draw API
