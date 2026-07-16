# Changelog

## [Unreleased]

### Added
- **Viewport free zone:** `is_viewport_hovered` — mouse over empty central dock (PassthruCentralNode) for 3D input.
- **Spectrum theme (Adobe-inspired):** `style_colors_spectrum` / `style_colors_spectrum_light` on host + Ori API.
  Default on raylib/GLFW init; Studio View menu switches Spectrum dark/light and stock ImGui themes.
  Palette from adobe/imgui Spectrum colors (no fork, no restricted fonts).
- **Studio dock layout (Godot/Unity-like):** `dock_space_studio` + `apply_studio_dock_layout` (DockBuilder);
  Hierarchy | Viewport passthrough | Inspector/Tools · bottom Console/Output/Timeline/Nodes.
  Shell: View → Reset dock layout; panels consolidated (no more free-floating E/D windows).

## [0.5.1] - 2026-07-15

### Added
- **`imgui.test_harness` (Stage C4, pure Ori):** headless UI test hooks —
  register widgets by label (button/window/menu/checkbox/header/menu_item),
  `query_by_label` / `exists` / `kind_of` / open-checked-visible queries,
  `click` / `open_item` / `close_item`, enable/visible gates, frame counter.
- Tests: `tests/test_test_harness.orl` (4 cases, headless — no window).
- **Choice documented:** pure Ori harness — **not** full official
  `imgui_test_engine` CI / pixel regression.

## [0.5.0] - 2026-07-15

### Added
- **Multi-context (B3):** complete create / set / current / destroy APIs on the
  host + `imgui.ui` wrappers; `context_is_live` / `max_contexts` queries.
  Slot 0 is primary (from `init` / `init_raylib`); extras for editor vs game
  compositor. Primary cannot be destroyed via `destroy_context`; shutdown frees
  remaining extras.
- **`image` / `image_button`:** draw / click GPU textures by opaque int id
  (OpenGL / raylib texture id). `image_button(id, tex_id, w, h) -> bool`.
- Tests: `tests/test_context_image.orl` (≥4 cases).
- README: editor vs game multi-context section + image helpers.

### Fixed
- Link `imgui_demo.cpp` so `ShowStyleEditor` / Tier2 style editor resolve at AOT link.

### Added (prior unreleased, still shipping)
- **P2-A raylib embed:** `ori_imgui_init_raylib` / `begin_frame_raylib` /
  `end_frame_raylib` — ImGui into an existing raylib/`game.app` window
  (OpenGL3 backend; no second GLFW window). Ori: `init_raylib`,
  `begin_frame_raylib`, `end_frame_raylib`, `is_raylib_mode`.
- Demo: `demos/imgui_game/` (path deps `imgui` + `ori_game`). Smoke OK on Linux.
- **Tier 0 foundation:** docking (`dock_space_over_viewport`, `dock_space`),
  tables (`begin_table` / `table_*`), `selectable`, `columns` / `next_column`,
  `is_item_clicked`. Docking enabled on init. Demo shows dockspace + entity table.
- **Tier 1 editor essentials:**
  - `imgui.file_browser` — pure Ori + `ori.fs.list_dir` browser (filter by ext)
  - `imgui.inspector` — int/bool/milli property fields
  - `imgui.plot` — ring buffer + `PlotLines` (milli samples)
  - `imgui.nodes` — simple draggable node canvas + links (not full imnodes)
  - Host: popups, plot buffer, draw_line/rect, mouse down/release, cursor screen
  - Demo: `examples/tier1_demo.orl`
- Build: raylib embed is **opt-in** (`ORI_IMGUI_WITH_RAYLIB=1`); default GLFW-only
  so package smoke does not require raylib at link.

## [0.4.1] - 2026-07-15

### Added
- **`imgui.timeline` product surface (sequencer G1, pure Ori):** multi-track
  list, playhead milli (`play` / `pause` / `seek` / `tick` / clamp), key CRUD
  (`add_key` sorted by time, `remove_key`, `set_key`, `get_key`, `key_count`,
  `find_key_at`), track CRUD helpers, step `sample` + linear `sample_lerp`.
- Tests: `tests/test_timeline.orl` (≥4 cases). Smoke runs timeline + curves tests.
- **Choice documented:** pure Ori deepen — **no** new `ori-imsequencer` / C++
  ImSequencer package for maturity 5 Linux G1.

## [0.3.0] - 2026-07-14

### Added
- `slider_float_milli` — float slider via milli-int FFI (`value * 1000`)
- `combo` — up to 8 fixed string labels
- `begin_child` / `end_child` — bordered child regions
- Multi-slot `input_text`: `input_text_slot`, `input_text_multiline`,
  `input_text_select`, `input_text_slot_set` / `_value`, `input_text_clear`,
  `input_text_len`
- Expanded `examples/demo.orl` (combo, float slider, child, multiline)

## [0.2.0] - 2026-07-13

### Added
- menus, tree, sliders, progress, input_text, capture flags
- MVP host GLFW+OpenGL3
