# ori-imgui

[Dear ImGui](https://github.com/ocornut/imgui) for Ori (S3).

**Package:** `imgui` · **Modules:** `imgui.ui`, `file_browser`, `inspector`, `plot`, `nodes`, `curves`, `timeline`, `test_harness`  
**Version:** 0.5.1  

**Backends:**
- **GLFW + OpenGL3** — standalone (`init` / `begin_frame` / `end_frame`)
- **raylib embed (P2-A)** — `init_raylib` …; build with `ORI_IMGUI_WITH_RAYLIB=1`

| Tier | Modules / APIs |
|------|----------------|
| **0** | docking, tables, selectable |
| **1** | `file_browser`, `inspector`, `plot`, `nodes` (canvas MVP) |
| **2** | `curves`, **`timeline`**, **multi-context**, **`image` / `image_button`**, **`test_harness`** |

```bash
./tools/build_linux.sh
./tools/smoke_linux.sh   # demo + tier1 + timeline/curves + context/image + harness tests
```

**Requires:** Ori ≥ 0.3.0 · CMake · X11/OpenGL · (embed) raylib headers at build

## Build (Linux)

```bash
# first time: vendor cimgui+imgui (recursive) and glfw already expected under vendor/
./tools/build_linux.sh
./tools/smoke_linux.sh
```

Artifacts in `lib/x86_64-unknown-linux-gnu/`:

| Lib | Role |
|-----|------|
| `libori_imgui.a` | host C API + imgui core + backends |
| `libglfw3.a` | vendored GLFW |
| `libsysdeps.a` | ld script: GL, libstdc++, X11, … |

## Usage — standalone GLFW window

```ori
import imgui.ui as ui

main()
    if not ui.init(480, 320, "My UI")
        return
    end
    while not ui.should_close()
        ui.begin_frame()
        if ui.begin_window("Panel")
            ui.text("Hello")
            if ui.button("OK")
                -- …
            end
            ui.end_window()
        end
        ui.end_frame()
    end
    ui.shutdown()
end
```

## Usage — embed in `game.app` / raylib (P2-A)

```ori
import game.app as app
import imgui.ui as ui

on_draw() -> void
    -- world draw first …
    ui.begin_frame_raylib()
    if ui.begin_window("HUD")
        ui.text("same window as the game")
        ui.end_window()
    end
    ui.end_frame_raylib()
end

main()
    -- init_raylib after InitWindow (e.g. first on_update)
    app.run_window("Game", 640, 360, on_update, on_draw)
    ui.shutdown()
end
```

Demo: `demos/imgui_game/` (`./tools/smoke.sh`).

## Widget / API table

| Function | Role |
|----------|------|
| `init` / `shutdown` | window + ImGui primary context (slot 0) |
| `should_close` | GLFW close flag |
| `begin_frame` / `end_frame` | NewFrame + Render + swap |
| `begin_window` / `end_window` | `Begin` / `End` |
| `button` / `small_button` | clickable |
| `image` / `image_button` | draw / click GPU texture by int id |
| `text` / `text_colored` / `bullet_text` | labels |
| `checkbox` | bool toggle |
| `slider_int` | int range |
| `slider_float_milli` | float via milli-int (`value * 1000`) |
| `drag_int` | int drag |
| `progress_bar` | fraction in milli (0..1000) |
| `combo` | up to 8 fixed string items |
| `input_text` / `input_text_slot` | single-line (4 slots) |
| `input_text_multiline` | multi-line slot |
| `input_text_set` / `input_text_slot_set` | seed buffer |
| `input_text_value` / `input_text_slot_value` | read via temp path |
| `input_text_select` / `input_text_clear` / `input_text_len` | slot helpers |
| `begin_child` / `end_child` | child region (optional border) |
| `collapsing_header` / `tree_node` / `tree_pop` | hierarchy |
| `separator` / `same_line` / `spacing` | layout |
| `begin_main_menu_bar` / `begin_menu` / `menu_item` | menus |
| `want_capture_mouse` / `want_capture_keyboard` | input routing |
| `set_next_window_size` / `set_next_window_pos` | window placement |
| `style_colors_spectrum` / `_light` | Adobe Spectrum-inspired theme (Studio default dark) |
| `create_context` / `set_current_context` / `current_context` / `destroy_context` | multi-context |
| `context_is_live` / `max_contexts` | context pool query |

Demo: `examples/demo.orl`

## Multi-context — editor vs game

Dear ImGui is **global-current-context**: widgets always go to `current_context()`.
`ori-imgui` keeps a small slot table (default max 4):

| Slot | Role |
|------|------|
| **0 (primary)** | Created by `init` (standalone GLFW editor host) or `init_raylib` (game embed). **Owned by** `init`/`shutdown` — cannot be destroyed via `destroy_context`. |
| **1..N** | Extra isolated contexts via `create_context()` (e.g. a second tool surface, preview UI). |

**Editor (standalone GLFW):** call `ui.init(...)` once. Most apps stay on slot 0.
Optional extras:

```ori
var tool: int = ui.create_context()   -- slot >= 1
ui.set_current_context(tool)
-- … build tool widgets …
ui.set_current_context(0)             -- back to editor primary
ui.destroy_context(tool)
```

**Game (raylib / `game.app` embed):** call `ui.init_raylib()` after the game window
exists. Slot 0 is still primary and shares that GL context. Use the same
`set_current_context` API if you need an isolated overlay context.

**Compositor tips:**
1. Always `set_current_context` **before** `begin_frame` / widgets for that surface.
2. Destroy extras before `shutdown` (shutdown also frees remaining extras).
3. Do not destroy slot 0 manually — use `shutdown`.

Tests: `tests/test_context_image.orl` (≥4 cases; needs a display for GLFW init).

## Image / texture id

`image` and `image_button` take an **opaque int texture id** (OpenGL `GLuint` or
raylib `Texture.id` cast to `int`). Size is pixel width/height. No UV/tint in this
MVP — pass a full texture.

```ori
ui.image(tex_id, 64, 64)
if ui.image_button("pick", tex_id, 32, 32)
    -- clicked
end
```


## Timeline / sequencer (`imgui.timeline`) — pure Ori

**Product choice (maturity 5 G1):** deepen pure Ori `imgui.timeline` instead of a
new `ori-imsequencer` C++ ImSequencer package. Tracks + playhead milli + key CRUD
meet the tools plan **B2** surface without FFI.

| Function | Role |
|----------|------|
| `create` / `set_duration` / `duration` | timeline shell (duration milli) |
| `add_track` / `remove_track` / `track_count` / `track_name` / `rename_track` | tracks |
| `add_key` / `remove_key` / `set_key` / `get_key` / `key_count` / `find_key_at` | key CRUD (times sorted) |
| `play` / `pause` / `seek` / `tick` / `playhead` / `is_playing` | playhead milli |
| `sample` | step hold at playhead |
| `sample_lerp` | linear milli lerp between keys |

Headless tests: `tests/test_timeline.orl`. No window required for the model API.

```ori
import imgui.timeline as tl

main()
    var t: tl.Timeline = tl.create(1000)
    t = tl.add_track(t, "x")
    t = tl.add_key(t, 0, 0, 0)
    t = tl.add_key(t, 0, 1000, 1000)
    t = tl.seek(t, 500)
    -- sample_lerp -> 500
end
```

## Test harness (`imgui.test_harness`) — pure Ori

**Product choice (Stage C4):** a small **pure Ori** label registry + click/open
simulation for headless editor/UI logic tests. **Non-goal:** full upstream
[imgui_test_engine](https://github.com/ocornut/imgui_test_engine) CI, mouse
injection into the host, or pixel regression.

| Function | Role |
|----------|------|
| `create` / `clear` / `begin_frame` | harness lifecycle |
| `register_*` / `register` / `unregister` | button, window, menu, checkbox, header, menu_item |
| `exists` / `query_by_label` / `kind_of` / `label_at` | query by **exact label** |
| `is_open` / `is_checked` / `is_enabled` / `is_visible` / `click_count_of` | state query |
| `click` | simulate click (toggle checkboxes; open closed windows/menus/headers) |
| `open_item` / `close_item` | force open/close openable kinds |
| `set_checked` / `set_enabled` / `set_visible` | seed / gate interaction |

Headless tests: `tests/test_test_harness.orl` (no GLFW window).

```ori
import imgui.test_harness as th

main()
    var h: th.Harness = th.create()
    h = th.register_button(h, "OK")
    h = th.register_window(h, "Panel", false)
    h = th.click(h, "OK")
    h = th.open_item(h, "Panel")
    -- th.click_count_of(h, "OK") == 1
    -- th.is_open(h, "Panel")
end
```

Does **not** change multi-context APIs from **0.5.0** (`create_context` / …).

## Phase OS

**Linux-first.** Windows/macOS native host builds are **deferred** (Phase OS last —
non-blocking for maturity **5** / package **0.5.1**).  
MSVC host stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (stub host; full GLFW optional).  
Timeline / curves / `test_harness` pure Ori modules need no native rebuild per OS.  
See ori-lang `docs/planning/PHASE-OS.md`.
