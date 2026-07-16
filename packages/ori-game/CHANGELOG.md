# Changelog

## [Unreleased]

### Changed
- **ADR Studio ImGui fino** (`ADR-2026-07-15-STUDIO-IMGUI-FINO.md`): stay on ImGui (no fyrox-ui); default Hierarchy|Viewport|Inspector|Toolbar; dock L/R only; F6 minimal.

### Fixed
- **Studio unusable lag (deep fix):** Godot-style **path cache** for `LoadModel`/`LoadTexture` in `ori_raylib_shim`; editor **lite** defaults (unlit cubes, no GLB until toggled, fewer panels); no GLB load on scene open/select; `draw_level_pieces_opts`; CSG only on `refresh_solids`. See `docs/planning/STUDY-TO-ORI-PERF.md`.
- **Studio/sample frame hitch:** `draw_level_ex` re-ran CSG `collect_solids` every frame; hot path is now `draw_level_pieces` + cached solids.

### Added
- **Studio Bake button:** Toolbar **Bake** + Tools → Nav bake; Play enemy **path_follow** from bake (goal/player) with box fallback; smoke shell package mode + recast path-dep.
- **Recast bake residual (GP4):** `game.nav_bake_geom` (pure top-face milli) + `game.nav_bake` (solids → triangle soup → `build_mesh`); path → `path_follow`; smoke `examples/nav_bake_smoke.orl`; pure tests.
- **R7.2+ enet wire:** `examples/world_net_sync.orl` — host sends `ws:` world_replicate payload; client `apply_message` (loopback). Path-dep `enet` on ori_game.
- **R7.2+ world replicate:** `game.world_replicate` — capture/encode/decode/`apply_snap` of up to 4 entity xyz (`ws:…` text); tests + `examples/world_replicate_smoke.orl`.
- **Studio F6 A/B fps:** rolling avg sample; on F6 toggle logs previous mode avg; toolbar + MINIMAL HUD show `avg≈` and last mode.
- **G1 content loop:** `engine_test` enemy (`inst_enemy_01`) + GP4 `path_follow` patrol (AI chase overrides in aggro); contact combat; win → coin **save slot** (`/tmp/ori_engine_test_g1.json`); HUD lines fixed (cam / fps / coins+save); enemy draw red.
- **R5.2 camera drive:** `game.camera_drive` keyframed offset clip (lerp + loop); sample/shell key **T**; tests `test_camera_drive`.
- **G1 sample polish:** debug off default; smaller grid; **R** restart; fps HUD; unlit solids.
- **R5.1 multi-cam stack:** `game.camera_stack` (follow/fixed slots, cycle, `resolve_pose` pure); sample + Studio Play (**V** / **1–4**); tests `test_camera_stack`.
- **P1/P3 OS polish:** `smoke_eco_windows.ps1` (ok/fail/skip, `-ValidateOnly`); `smoke_macos.sh` + `smoke_eco_macos.sh`; `validate_os_scripts.sh`; jolt/box2d Win smoke → `dynamics.orl`; export triple detect Darwin.
- **T4 dual viewport:** Studio shell 3D|2D (tilemap solids + level xz top-down + pan/zoom).
- **L6 CSG deep:** `aabb_intersect` / `union_hull` / boolean ops / adjacent merge / axis BSP clip (`game.ogame_level_csg`); `collect_solids` uses `carve_merged`; tools intersect + world plane; tests `test_csg_l6`.
- **R2.3 Play freshness:** `game.play_freshness` (content fingerprint vs `.ori_play_stamp`); shell warns on Play; menu Write/Check stamp; `tools/write_play_stamp.sh`.
- **Deep Linux:** L5 terrain raise/create tools; V3 GPU preview (`ori_rl_GetTextureGlId` + ImGui image); `game.path_follow` + sample; `physics_host_sample`; `enet.tf_codec`.
- **Linux residual:** V3 asset probe, L4 clip, R7 session/tf_sync, GP4 nav_path; world tests fixed.
- **Fase K (Linux MVP):** `asset_paths`, `postfx`, `decals`, `audio_buses`; sample `blip.wav` + inventory coins + particles/decals on win; shell Look toggles.
- **S5.1:** product entry `ori-studio/` (`tools/run.sh`, `smoke.sh`, README).
- **T3:** `examples/sample_2d_walk.orl` (tilemap solids walk).
- **R3.4 / R1.3:** box/capsule resolve radius; controller debug draw (sample key 1).
- **P4 CI Linux:** `tools/smoke_ci_linux.sh` · cluster `scripts/smoke_ci_linux.sh` · GHA · `bootstrap_path_deps.sh`.
- **V2.2 gizmo camera:** `game.camera_matrix` (look-at + perspective milli 4x4).
- **P2 export:** `tools/export_game_package.sh` — AOT bin + assets + native libs + `run.sh` (Linux first).
- **S3.5:** recent projects list (session) no Project panel.
- **Fases G→L (MVP):** Studio author loop + runtime gameplay hooks.
  - Shell: place instance, inspector rot/scale/overrides, open project, save current path, toolbar, Ctrl+P palette, viewport pick, frame (F), brush hierarchy + subtract, Play combat/AI/HUD.
  - Runtime: `character_controller`, `collision_layers`, `world_triggers`, `world_gameplay`; scene `make_instance`/`set_*_override`; level `add_subtract_box`; multi_scene stack; tilemap `serialize_compact`; world parent + raycast_spheres.
  - Tests: `tests/test_phase_g_l.orl`. Sample HUD + combat/AI optional.
- **Sample win condition:** `inst_goal` + reach radius in `engine_test` (`YOU WIN` HUD).

### Changed
- **`game.app.run_window` / `run_window_clear`:** set `FLAG_WINDOW_RESIZABLE` before `InitWindow` (OS maximize / resize).
- **Author loop polish:** `write_scene_with_level` / `serialize_scene_with_level` (instances + brushes/subtracts/terrain). Studio save keeps extrude. Sample root auto-discover (cwd monorepo vs shell). `room1` playable floor/walls + mesh overrides.
- **Box2D ↔ World:** `box2d.world_sync` + `tools/smoke_world_sync.sh` (path-dep full solver).

### Changed
- **Import clash fix:** `box2d.world` → `box2d.dynamics`; `jolt.world` → `jolt.dynamics` (Ori last-segment search vs `game.world`).
- **Fase E (author tools):**
  - `game.ogame_level_tools` — grid snap + face extrude/push-pull + add brush (tests `test_level_tools`).
  - Timeline/curves no shell via `imgui.timeline` + `imgui.curves`.
  - Tilemap paint/fill/erase + `collect_solid_rects` (`test_tilemap_paint`).
  - UX E4: `ori-imgui-extras` notify / search / command palette no `studio_shell`.
- **Fase F (engine broaden, Linux first):**
  - `game.world_physics` + `world_systems.physics_system` / `tick_world_physics` (optional PhysicsHost).
  - `game.multi_scene` — named scene slots + load/switch (`test_multi_scene`).
  - Shell panels: Level tools, Timeline, Tilemap, Toasts/Palette/Search, Multi-scene + Play gravity.
  - Net: `ori-enet` smoke 5 demos remains the net sample gate.
  - flecs **not** adopted — ECS-lite default (explicit F decision).
- **B1 — `game.world` ECS-lite:**
 `EntityId` + components (`Transform`, `Velocity`, `MeshComp`, `ColliderComp`, `ScriptComp`, `EntityMeta`) + dense `World` store; `spawn_scene` / `spawn_scene_path` from catalog+scene JSON; helpers `load_sample_world` / `load_engine_test_world`. Tests: `tests/test_world.orl`. `EntityId` alias also on `game.ids`.
- **B2/B3 — ordered systems + script host:** `game.world_systems` — fixed tick order `input → script_create → script_step → move → collision`; `ScriptHost` for ogs Create/Step. Sample `engine_test` `main.orl` uses World + systems. Tests: `tests/test_world_systems.orl`.
- **B4 — camera follow:** `game.camera3d.follow_offset`; sample follows player (arrows nudge offset).
- **ECO PR17 — optional package wires deepen (Linux):**
  - `game.gltf` / `game.obj`: mesh export helpers from cgltf/fast_obj **0.2** (interleaved buffers, materials, node TRS).
  - `game.physfs_assets`: write-dir / mkdir / write + multi-mount re-exports (physfs **0.2**).
  - New thin modules: `game.noise` → `noise.fnl`, `game.compress` → lz4 + miniz, `game.navmesh` → `recast.nav` (leaf unique vs package).
  - Path deps in `ori.pkg.toml`: cgltf/fast_obj/physfs **0.2.0**; noise/lz4/miniz/recast **0.2.0**.
  - Tests: `test_gltf`, `test_obj`, `test_physfs_assets`, `test_noise`, `test_compress`, `test_navmesh` (fixtures upgraded).
  - Still **no** `native_libs` on `ori_game` (path-dep only).

### Fixed
- **Sample Play SIGSEGV (window flash-close):** returning `TickResult { World, ScriptHost }` in one AOT return corrupted managed aggregates. Split into `tick_scripts` + separate `input`/`move`/`collision` calls (no dual-struct return).
- Inherit `raylib` `native_libs` from path-dep only (avoid dual-link of `ori_rl_*`).

### Added
- **G1 B5–B8:** collision tests (`collision_system` push + layer + engine_test wall); sample wall brush + mesh fallback HUD; guide `docs/ENGINE-TEST-GUIDE.md`.
- **Studio C0–C6 (Fase C completa):** `ori-imgui/demos/studio_shell` — docks, Hierarchy, Inspector, Viewport 3D, **Play in-process**, Console logs, **Open script** (`app.open_path` / raylib OpenURL). Decisão **GO** ImGui: `docs/planning/C6-IMGUI-SHELL-GO.md`.
- **raylib shim:** `ori_rl_OpenURL` / `ori_rl_OpenPath` (file://).
- **`game.ogame_scene`:** `serialize_scene` / `write_scene` / `make_scene` (instance round-trip; brushes full writer later).
- `tools/smoke_eco_linux.sh`: continue after package failures; restore full raylib after stub smoke; `ECO_SMOKE_SKIP_GAME` / `ECO_SMOKE_SKIP_DEMOS`.


### Fixed
- **Sample Play opens a game window:** `main()` now calls `game.app.run_window` (was empty → silent exit). Verified raylib 5.5 InitWindow 1280×720.
- **`native_libs` on sample/scaffold without staged `lib/`:** removed; libs resolve via path-dep `ori_game`/`raylib` `.a`. Studio Play forces `ORI_USE_AOT=1` (JIT needs `.so` not staged).
- **New Project scaffold:** windowed `main.orl` + package without false `native_libs`.

### Changed
- **`docs/planning/PLANO-IMPLEMENTACAO-STUDIO.md`:** reescrito para visão ECS-lite/híbrido, Studio ori-imgui destino, IDE externa, fases A–F / G1 (Tauri = transição).
- **`STUDIO-PRODUCT-DECISIONS.md` + `ORI-GAME-STUDIO-VISAO.md`:** alinhados à visão 2026-07-15.

### Added
- **`docs/planning/ROADMAP-GAME-ECO.md`:** full Dear ImGui extensions catalog (all listed libs, links, P0–P4, packaging) + link to `IMGUI-EXTENSIONS-RANKING.md`.
- **Optional path-dep wires (ECO PR8):** `cgltf` / `fast_obj` / `physfs` path deps in `ori.pkg.toml`; thin helpers `game.gltf` (`cgltf.loader`), `game.obj` (`fast_obj.mesh`), `game.physfs_assets` (`physfs.fs`). Fixtures under `tests/fixtures/`; tests `test_gltf`, `test_obj`, `test_physfs_assets`.
- **`docs/planning/PLANO-AMADURECIMENTO-ENGINE.md`:** full maturation plan — GM/Unity/Godot philosophy & architecture, complete missing-features backlog, phases A→E, gate **G1 first game**, ImGui Studio decision, implementation queue.
- **`docs/planning/STUDY-ENGINES.md`:** study protocol; Godot source at `Documentos/Projetos/study-godot` (shallow clone).
- **`docs/planning/PRODUCT-REALITY.md`:** honest maturity vs “done MVP”; recovery plan P0–P3 (not a finished GM/Godot/Unity).
- **`docs/planning/PLANO-AMADURECIMENTO-ENGINE.md`:** full maturation plan — GM/Unity/Godot architecture study, complete backlog, phases A→E, gate G1 first game, ImGui Studio decision.
- **`docs/planning/STUDY-ENGINES.md`:** study protocol; Godot shallow clone at `Documentos/Projetos/study-godot`.
- **Sections 1–2 complete (Linux):** FreeType atlas→texture (`game.font_atlas`); HarfBuzz AOT+layout; MC `export_obj`/`marching_cubes_gpu`; raylib gray texture + mesh + music seek; ImGui multi-context + style/image/curves/timeline; audio buses/pool; dialogue choices; inventory remove/use; combat i-frames; AI patrol; camera limits/shake; save slots; `game.actions` / `pathfind` / `cutscene` / `net_predict` / `shader_presets`.
- **`game.ogame_script`:** runtime interpreter for Studio N1 `ogs:*` markers (print, set_var, if, wait, alarm, call); milli-int vars/wait; tests `tests/test_ogame_script.orl` (5). Sample `main.orl` hosts player CREATE_OGS on load.

- **`game.tiled.solid_tiles` / `first_layer_solid_tiles`:** map solid GIDs → world boxes (Y-up) for level/collision wiring; test in `test_tiled`.
- **Play packaging docs:** `docs/planning/PLAY-PACKAGING.md`; smoke `tools/smoke_sample_play.sh` asserts path-dep + package check.
- **Studio ECO hooks doc:** `docs/planning/STUDIO-ECO-HOOKS.md` (surfaces tiled/aseprite/level/nodes ↔ packages).
- **Scene player helpers:** `game.ogame_scene.first_of_entity`, `override_float` / `override_int` / `override_string`.
- **sample `main.orl`:** loads player position + vars (`move_speed`, `mesh_scale`, `collider_radius`, `collision_layer`, `mesh_path`) from `engine_test` instance.
- **sample `engine_test` scene:** default Play/Studio test (`active_scene`); floor box brush; player = brazilian-cowboy GLB (`assets/models/brazilian-cowboy.glb`); `main.orl` WASD + layer collision + mesh draw; mesh_scale default **48**.
- **sample package:** `ori.pkg.toml` path-dep `ori_game` so Play/`ori check .` resolves `game.*`.
- **Level deepen:** `resolve_sphere_on_layer`; brush `albedo_path` + mesh scale/yaw via `draw_mesh_at` / `draw_model_ex`; Studio `albedoPath` on brushes.
- **`game.marching_cubes`:** classic Marching Cubes (milli-int grid → mesh triangles); tests `tests/test_marching_cubes.orl`.
- **`game.marching_cubes` complete surface:** `mesh_bounds` / `Bounds3m`, `flatten_positions_m`, `flatten_indices`, `flatten_wire_segments_m`, `has_surface`, `vertex_count`, `m_to_f`; draw helper `game.marching_cubes_draw` (`draw_mesh_wires` / `draw_mesh_solid`) via `draw3d.draw_line_3d` / `draw_triangle_3d` (raylib `DrawLine3D` / `DrawTriangle3D`).
- **Level CSG multi-piece + rotation:** `game.ogame_level_csg` (oriented OBB → world AABB, sequential AABB carve → N slabs); `collect_solids` carves every box/plane/ramp brush by all subtracts (not center-drop). Tests: partial carve, multi-cut, oriented expand, `resolve_sphere`.
- **PBR materials (raylib):** shaders `assets/shaders/pbr_lit.{vs,fs}`; `ogame_level_draw.draw_level_ex` binds metallic/roughness/light/view per solid; `SolidPiece` carries metal/rough; sample project ships shader copies.
- **Collider debug + player query:** `draw_level_ex(..., debug_colliders)` / `draw_collider_debug`; sample `main.orl` WASD player sphere uses `resolve_sphere` against CSG solids; **C** toggles wires; keys `KEY_C` / `KEY_LEFT_SHIFT` on `game.input`.
- **Terrain columns** still feed solids for collision/draw (block heights from scene JSON).

### Changed
- **Level modules readability:** `collect_solids` → `append_brush_solids` / `append_terrain_columns` / `solid_from_brush_geom`; CSG `oriented_to_aabb` uses `fmin`/`fmax` + `rotate_local_corner`; draw path via `draw_cube_lit` / `begin_pbr`; sample `main` helpers `nudge_cam` / `axis_from_keys` / `update_player`.
- **Clean-code domain types (level/CSG):** `OrientedBox`, `Bounds3`, `oriented_box_to_aabb`, `carve_oriented`; collision via `expanded_collision_bounds` + `push_out_shallowest` + axis constants; draw via `PbrParams` / `draw_solid_piece` stepdown.

### Added (prior)
- **`game.ogame` (R0–R3 Studio content):** parse/load `entity.json` +
  `scene.json`; catalog + `spawn_scene` / var merge / `spawn_into_engine`.
  Fixtures `assets/sample_project/`; tests `tests/test_ogame.orl` (8);
  headless `examples/ogame_scene_smoke.orl`.
- **Clean-code split:** `game.json_fields` (helpers JSON compartilhados);
  `ogame_entity` / `ogame_scene` / `ogame_spawn` + facade `game.ogame`.
- **`examples/cangaceiro_demo.orl`:** load/orbit low-poly cangaceiro OBJ + diffuse
  texture via `game.draw3d` / `game.draw` (`assets/cangaceiro/`); procedural
  clips (Idle/Walk/Spin/Bow) + **In place** checkbox UI.
- **`game.anim_player`:** skeletal or procedural `ClipPlayer`, `in_place`,
  `set_clip`, `tick(dt)`, `draw_posed`.
- **`draw3d.update_model_animation_in_place`:** zero root bone XZ (skeletal).
- **GPU skinning:** `update_model_animation_bones` (+ in_place), 
  `model_set_all_materials_shader`, `model_supports_gpu_skinning`;
  shaders `assets/shaders/skinning.{vs,fs}`; `anim_player` flag `gpu_skinning`.
  Demo `cangaceiro_demo` enables GPU path when the GLB has bone matrices.
- **`tools/obj_inject_normals.py`:** inject face `vn` into Wavefront OBJ so
  raylib 5.5 `LoadModel` does not SEGV on Blender/Tripo `f v/vt` exports.
- **`game.tiled` (P1-A):** Tiled Map Editor JSON loader (`parse_map` / `load_map`),
  layers, object groups, `layer_to_tilemap`. Sample `assets/maps/sample.tmj`,
  demo `examples/tiled_demo.orl`, tests `tests/test_tiled.orl` (4 cases).
- **`game.aseprite` (P1-C):** Aseprite JSON sheet + tags + player tick. Sample
  `assets/sprites/hero.json`, demo `examples/aseprite_demo.orl`, tests
  `tests/test_aseprite.orl` (3 cases).
- **P2-B audio deepen:** master volume, sound pitch/pan/pause/resume/playing,
  music pause/resume/pitch/time length & played. Demo `audio_deepen_demo.orl`.
- **P2-B 3D deepen:** cylinder/capsule (+ wires), model wires, billboard.
  Demo `shapes3d_demo.orl`.
- **P2-C rres ↔ assets:** `game.rres_assets` (ORPK → extract → `AssetCache`);
  `asset_loader.load_*_as` logical keys. Demo `demos/rres_assets`, test
  `test_asset_loader_as`. Path-dep `rres` on package for import resolution.
- **P1-A2 `game.ldtk`:** LDtk project JSON — IntGrid, Entities, Tiles; sample
  `assets/maps/sample.ldtk`, demo `ldtk_demo.orl`, tests `test_ldtk.orl` (4).
- **`ori-raylib` split (B2.12):** L0 `raylib` package sibling; `tools/setup_raylib_*`
  wraps and stages into `ori-game/lib/`. Path-dep `raylib = { path = "../ori-raylib" }`.
- **P1-C2 `game.spine` (deepen):** Spine JSON bones + rotate/translate timelines.
  Pose storage is **milli-int** (avoids `list[float]` codegen residual). Player
  `update_player` / `world_bones` (fixed-point cos/sin). Sample
  `assets/sprites/hero_spine.json`, demo `spine_demo.orl` (idle/walk), tests
  `test_spine.orl` (3 cases).
- **`docs/planning/ROADMAP-GAME-ECO.md`:** queue + monorepo note + ImGui extensions
  roadmap for future native Studio.
- **`tools/smoke_eco_linux.sh`:** umbrella smoke for all ECO packages + integration demos.
- **`docs/planning/PHASE-OS.md`:** multi-OS (Win/mac) follow-up program after Linux-5.

## [0.3.0] - 2026-07-14

### Added
- **Gamepad L0/L1:** `ori_rl_IsGamepad*` / axis + `game.input` wrappers and
  constants; demo `examples/gamepad_demo.orl`.
- **RenderTexture L0/L1:** load/unload/begin/end/draw + rec; `game.draw`
  helpers; demo `examples/render_texture_demo.orl`.
- **S3 demos (Wave 1):** platformer, tilemap, particles, physics, inventory,
  dialogue, state, audio, ai, space_shooter rewritten for current `game.*`.

### Fixed
- **`assets/cube.obj`:** added `vn` (raylib 5.5 LoadOBJ requires normals).
- **raylib shim float ABI:** `ori_rl_*` now uses C `double` for all floating
  args/returns (Ori `float` = f64). Previous `float` (f32) mis-read SSE args
  (often as 0) → black 3D camera / invisible draws.

### Changed
- **Package version 0.3.0** — Linux product maturity target for core `ori-game`
  (gamepad + RT + full S3 demo set). Multi-OS still later.
- **S3 demo hygiene:** no remaining `import ori.game` / `namespace` demos.
- **`tools/smoke_linux.sh`:** expanded check + compile/run list (migrated demos,
  gamepad, render texture).
- **`game.mechanics.ai`:** renamed local `len` → `dist_sq` (avoid `ori_len` codegen clash).

## [0.2.0] - 2026-07-13

### Added (trilha R + L1 complete — 2026-07-13)
- **L1 2D complete** for existing L0 surface:
  - `game.draw`: line_ex, triangle, poly, text_ex, measure_text, font_default,
    load/unload texture, draw_texture / _v / _rec, texture_width/height
  - `game.input`: key_up, next_key, mouse_pos, mouse_delta, mouse_released
  - `game.audio`: full music stream L1 (load/play/stop/update/volume/playing)
  - `game.app`: fps/time/screen/fullscreen/config/exit key + begin/end_drawing escape hatch
- **`game.ids`**: `TextureId`, `ModelId`, `SoundId`, `MusicId`, `FontId` aliases
- **Raylib 3D (same dual surface as 2D):**
  - L0 `raylib`: all symbols **`ori_rl_*`** (no clash with raw raylib)
  - L1 `game.camera3d` + `game.draw3d` (snake_case, `Vec3`, `Color`, `ModelId`)
- **R1 shim:** `native/ori_raylib_shim.c` (handle tables + pack/unpack) +
  `tools/raylib_stub.c` headless; `native_libs = ["ori_raylib_shim", "raylib"]`
- Docs: `docs/ffi-3d.md`, `docs/api-3d.md`; sample `assets/cube.obj`
- Example `examples/hello_3d.orl` (grid/cube/sphere + optional model); `tests/test_l1_api.orl`
- **R3:** `game.ray3d` (mouse ray, hit sphere/box/ground, last_hit, draw_bounding_box);
  L0 `ori_rl_GetMouseRay` / `RayHit*` / last-hit getters; example `pick_3d.orl`
- **R3 residual:** model diffuse texture + model animations L0/L1
  (`set_model_diffuse_texture`, `load/update/unload_model_animations`)
- **Shaders / lights:** `game.shader`, `game.light`, `assets/shaders/lighting.*`,
  demos `shader_light_demo`, `skeletal_anim_demo`
- Skeletal helpers: `model_animation_frame_count`, `next_animation_frame`
- **Light bank (4 slots):** `game.light_bank` + `assets/shaders/lit_bank.*` +
  `light_bank_demo`
- **Anim clip player:** `game.anim_player` + `anim_player_demo`

### Added (trilha O — 2026-07-13)
- `game.tween`, `game.scene`, pure `game.assets` + `game.asset_loader`
- `game.save` file helpers (`save_json` / `load_json` / `is_valid_json`)
- Tests: tween, scene, assets, save file roundtrip
- Example: `examples/scene_menu.orl`


### Changed
- **S3 surface:** all library modules use `module`, `import path = alias`,
  `public` declarations, braced struct literals, and `list[T]`.
- Modules renamed to `game.*` / `raylib` (no `ori.game.*` package path).
- Engine/helpers rewritten for return-style mutation (no field defaults, `nil`, or bitwise).
- Package `ori_version` raised to `0.3.0`.

### Added
- `game.app.run_window` / `run_window_clear` loop helpers.
- Linux `tools/setup_raylib_linux.sh` (real raylib 5.5 build or `--stub`).
- `tools/smoke_linux.sh` for check + tests + canonic demos.
- Unit tests: shapes, engine, physics, inventory, save.
- Canonic examples: `hello_game`, `app_smoke`, `simple_game`.

### Notes
- Other examples under `examples/` may still be pre-S3; migrate later.
- JSON/FS save, raygui, Box2D, Jolt are **not** in this release (later ECO trails).


All notable changes to ori-game will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- **Ori Game Studio (planejamento + agentes):** skill `ori-game-studio` (pack
  grok-memory + `~/.grok/skills` + `.grok/skills`), agents `ogs-runtime` /
  `ogs-studio-ui` / `ogs-graph`, `AGENTS.md`, plano
  `docs/planning/PLANO-IMPLEMENTACAO-STUDIO.md` (React confirmado, stack de
  libs auditada, módulos de mercado).
- **Linux raylib setup:** `tools/setup_raylib_linux.sh` + headless `tools/raylib_stub.c`
  (linkable without X11; real desktop needs `libraylib-dev` + X11/GL).
- **`game.app`:** `run_window` / `run_window_clear` high-level loops.
- **Engine:** real gravity/movement `tick`, `cleanup_dead`, `set_entity`, tags;
  removed function-pointer fields (native codegen).
- **Particles:** `update_particles`, `draw_particles`, `create_emitter`.
- **Camera:** follow lerp, limits, shake, `update_camera`.
- **Save:** JSON slots via `ori.fs` (`save_game` / `load_game` / `has_save`).
- **Physics:** pairwise body resolve inside `step_physics`.
- **Tests:** engine, physics, inventory, save (+ shapes); all with `@test`.
- **Smoke:** `tools/smoke_linux.sh` (check + test + compile/run demos).
- **Docs:** rewritten README, getting-started, api.md; analysis doc updated.

### Changed

- Inventory usable items use `use_effect: string` instead of function callbacks.
- Modernize for Ori compiler current surface: namespaces `game.*` / `raylib`
  (no longer `ori.game.*`, which collides with stdlib classification).
- Syntax: `pub` (not `public`), typed `const`/`var`, `--` comments, no field
  defaults, no bitwise ops, FFI mouse as `GetMouseX`/`GetMouseY`.
- Mutation: free functions that change structs return the updated value
  (Ori parameters are immutable).
- **All library modules** pass `ori check` (28 files): core, physics, tilemap,
  dialogue, state_machine, save, and all `game/mechanics/*`.
- Style: return-value mutation, full struct literals, reserved-word fields
  renamed (`loop`→`does_loop`, param `map`→`tilemap`, local `result` avoided),
  `map<K, V>` / `list of T`, enum match with `case Variant:`.
- `raylib` extern members are `pub` so demos can call them across modules.
- **All examples** (15) and **tests** (2) pass `ori check`.
- Inventory API expanded: `create_item`, `create_usable_item`, `add_item`,
  `count_item`, `use_slot`.
- Physics: `set_body` for per-body updates after forces.
- Demos use explicit raylib loops, named callbacks (no `do() =>`), typed
  bindings, and current `game.*` modules.

## [0.1.0] - 2026-07-05

### Added

- Initial release of ori-game engine
- Raylib bindings for window, shapes, textures, text, input, and audio
- Core engine with Entity, Timer, Systems, and Game Loop
- Shape structs (Rectangle, Circle, Triangle, Line, Polygon)
- Color system with predefined colors
- Collision detection (primitives + SAT for polygons)
- Camera system with follow, shake, and limits
- Input helpers for keyboard and mouse
- Drawing helpers for common shapes
- Timer system with repeat and callbacks
- Physics 2D with gravity and collision resolution
- Tilemap support with chunks for large worlds
- Movement mechanics (4axis, 8axis, grid)
- Jump mechanics (coyote time, double jump, variable height)
- Dash mechanics with cooldown
- Combat system (health, knockback, knockdown, cooldowns)
- AI behaviors (patrol, chase, flee, wander)
- Behavior tree system
- Grab and drag mechanics
- Rope physics
- Vehicle mechanics
- Moving platforms
- Dialogue system
- Inventory system
- Particle system
- Save/Load system
- Audio manager with volume control and fading
- State machine for entity states
- Documentation (.oridoc files)
- Examples (hello_game, platformer)
