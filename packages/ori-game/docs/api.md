# ori-game API overview (current)

Namespaces use `game.*` / `raylib`. All mutators return updated values.

## game.world — ECS-lite store (G1, canonical)

The **current** world model (prefer over legacy `game.engine`).

| Function | Description |
|----------|-------------|
| `empty_world()` / `spawn_entity(w, meta, transform)` | Create / append entity |
| `spawn_scene(w, catalog, scene)` / `spawn_scene_path` | Spawn from scene JSON |
| `find_by_id` / `find_by_instance` / `find_first_of_type` | Queries (alive only) |
| `set_transform` / `set_velocity` / `set_mesh` / `set_collider` / `set_script` | Component writes |
| `despawn(w, id)` | Mark dead (`alive=false`) |
| `cleanup_dead(w)` | **Compact** dead entities (call periodically on spawn churn) |
| `set_parent` / `world_transform_of` | 1-level parent (translation only) |
| `raycast_spheres(w, ox..dz, max)` | Nearest collider hit (**dir must be unit**) |

Components on `Entity`: `Transform`, `Velocity`, `MeshComp`, `ColliderComp`,
`ScriptComp`, `EntityMeta`, `float_vars` — presence via `has_*` flags.

## game.world_systems — fixed-order G1 loop

`input_system → tick_scripts → physics_system → move_system → collision_system`
(`tick_world` / `tick_world_physics` bundle the order). `FrameInput` carries
resolved axes. Script/physics live in **parallel hosts** (`ScriptHost`,
`PhysicsHost` — see AOT note in source: never return two big aggregates).

## game.world_gameplay — combat / AI / waves

| Function | Description |
|----------|-------------|
| `attach_health` / `attach_ai` / `set_player` | Slot bookkeeping by EntityId |
| `ai_chase_system(host, w, dt)` | Chase player on XZ at **constant** slot speed |
| `contact_combat_system(host, w, hit_radius)` | Contact damage (skips dead AI) |
| `damage_entity` / `tick_i_frames` / `despawn_dead` | Health pipeline |
| `start_waves` / `tick_waves` / `wave_index_of` / `waves_finished` | Wave state |
| `spawn_wave_enemies(w, n, cx, cz, y, radius)` + `attach_new_enemies(host, w, ...)` | Wave respawn (two calls by design) |

## game.world_triggers / world_replicate

Triggers: AABB volumes + enter/exit events (`tick` + `collect_events`);
volume `layer` 0 = any entity, else matches `collider.layer`.
Replicate: `encode_world` / `apply_message` — text snapshot ≤4 poses (demo net).

## game.path_follow / nav_bake / nav_path / pathfind

- `path_follow`: fixed 4-waypoint XZ path; `desired_velocity_xz` returns
  **constant-speed** velocity toward current waypoint; `advance_if_arrived`.
- `nav_bake.bake_solids(solids, cell_m, agent_m)`: level solids → recast mesh;
  `path_follow_from_query` fills a PathFollow (first 4 corners — re-query on
  finish for longer paths, as studio_shell does).
- `pathfind`: grid A* 4-neighbour (pure Ori, no recast dependency).

## game.camera_stack / camera3d / camera_matrix / camera_drive

Multi-cam Play stack (`default_play_stack`: chase/high/side/overview,
`next_cam`, `resolve_pose`); 3D camera wrappers; view/proj matrices (milli
ints for gizmo FFI); keyframed camera drive clips.

## game.character_controller

Sphere controller: `create(radius, max_speed)`, `apply_position` (returns
resolved pos) + `step` (updates grounded/vertical_vel) — pass the **same
start coords** to both. When a host uses the controller for the player, do
not also integrate the player's input velocity (double-move).

## game.ogame_* — scene / entity / level authoring

`ogame` (scene+catalog façade), `ogame_level` (brushes, CSG solids,
`resolve_sphere_on_layer` — layer_mask equality, 0 = all), `ogame_level_tools`
(extrude/clip/carve/terrain), `ogame_script` (ogs subset), `ogame_spawn`.

## game.decals / postfx / light_bank / shader_presets

Decal pool (`spawn` / `tick` / `decal_at` — host draws, e.g. thin cylinder),
post-fx look state, light rigs, shader presets.

## game.play_freshness

Content fingerprint of play sources vs stamp file (`evaluate` / `write_stamp`)
— Studio warns when Play sources changed since last compile.

## game.app

| Function | Description |
|----------|-------------|
| `run_window(title, w, h, update, draw)` | Init window, loop, clear, call callbacks |
| `run_window_clear(..., r,g,b, update, draw)` | Same with custom clear color |

Callbacks are **named module functions** (`func update(dt: float) -> void`).

## game.engine

| Function | Description |
|----------|-------------|
| `create_engine()` | Empty entity store |
| `with_builtins(eng)` | Enable gravity + movement integration |
| `enable_gravity` / `enable_movement` / `set_gravity` | Flags |
| `spawn_entity` / `spawn_tagged` | Append entity |
| `get_entity` / `set_entity` | Access by index |
| `tick(eng, dt)` | Apply gravity + integrate position for alive entities |
| `destroy` / `cleanup_dead` | Mark dead / compact list |
| `entity_count` | Length |

**Entity:** `id`, `pos`, `vel`, `alive`, `has_gravity`, `has_collision`, `tag`  
No function-pointer fields (codegen limitation).

## game.audio (P2-B deepen)

| Area | API |
|------|-----|
| Device | `init` / `close` |
| Master | `set_master_volume` / `master_volume` |
| Sound | load/unload/play/stop/**pause/resume**, **pitch**, **pan**, `sound_playing` |
| Music | load/unload/play/stop/update/**pause/resume**, volume, **pitch**, `music_length` / `music_played` |

## game.draw3d (P2-B deepen)

Cube/sphere/plane/grid/ray + **cylinder**, **capsule** (+ wires), **model_wires**, **billboard** (needs `camera3d` + texture id).

## game.input / draw / color / shape / collision

Typed wrappers over raylib / geometry. See source for full list.  
Input exposes `KEY_*`, mouse helpers, and **gamepad** (`gamepad_available`,
`gamepad_button_*`, `gamepad_axis`, `GAMEPAD_*` constants).  
Draw: `draw_rect`, `draw_circle`, `draw_text`, textures, and **RenderTexture**
(`load_render_texture`, `begin_texture_mode` / `end_texture_mode`,
`draw_render_texture` / `_rec`).

## game.tiled (P1-A)

Tiled Map Editor **JSON** (`.tmj` / JSON export). Orthogonal maps only.

| Function | Description |
|----------|-------------|
| `parse_map(text)` / `load_map(path)` | `result[TiledMap, string]` |
| `first_tile_layer` / `first_object_group` / `layer_by_name` | Layer lookup |
| `layer_to_tilemap` / `first_layer_to_tilemap` | → `game.tilemap.Tilemap` |
| `objects_of_type` / `object_rect` | Object groups |
| `is_solid_gid` | GID ≠ 0 convention |

Not yet: infinite maps, base64 chunks, tileset image atlases, LDtk.

## game.aseprite (P1-C)

Aseprite **JSON** spritesheet export (array or hash `frames` + `meta.frameTags`).

| Function | Description |
|----------|-------------|
| `parse_sheet` / `load_sheet` | `result[AseSheet, string]` |
| `frame_count` / `tag_count` / `tag_by_name` / `get_frame` | Lookup |
| `create_player` / `play_tag` / `tick` | Playback (`does_loop` flag) |
| `current_frame` / `current_source_rect` | For `draw.draw_texture_rec` |

Draw yourself: `draw.draw_texture_rec(tex, ase.current_source_rect(p), pos, tint)`.

## game.camera

| Function | Description |
|----------|-------------|
| `create_camera(offset, target)` | New camera |
| `follow(cam, target)` | Lerp toward target (+ optional limits) |
| `set_limits` / `set_zoom` / `set_follow_lerp` | Config |
| `shake(cam, intensity, duration)` | Screen shake |
| `update_camera(cam, dt)` | Decay shake |
| `begin_2d` / `end_2d` | Raylib mode |

## game.physics

World of `PhysicsBody`: `add_body`, `set_body`, `step_physics` (integrate + pairwise resolve), forces/impulses.

## game.tilemap

`create_tilemap`, `create_layer`, `add_layer`, `get_tile` / `set_tile`, `is_solid` / `is_platform`, `resolve_collision`, `check_platform`.

## game.particles

`create_particle_system`, `create_emitter`, `add_emitter`, `emit`, `update_particles`, `draw_particles`, `clear_particles`.

## game.inventory

`create_inventory`, `create_item`, `create_usable_item(id, name, use_effect)`, `add_item`, `count_item`, `use_slot`, `slot_effect`.  
Usable items use **string effects**, not function pointers.

## game.save

Checkpoints in memory + JSON slots via `ori.fs`:  
`save_game` / `load_game` / `has_save` / `delete_save`, `create_save_data`.

## game.dialogue / state_machine / mechanics.*

See `.orl` sources under `game/` and `game/mechanics/`.  
State machine / BT / dialogue callbacks must be **named functions** at module scope.

## raylib

Package entry + raw `extern c` surface used by wrappers. Link with `lib/<triple>/libraylib.a` via package `native_libs = ["raylib"]`.
