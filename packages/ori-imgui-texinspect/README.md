# ori-imgui-texinspect

**Texture inspect** for Ori (S3): show texture id, zoom, RGBA channel toggle.
Pure-Ori state machine with headless test hooks; product draw via sibling
[`ori-imgui`](../ori-imgui) (`imgui.ui`). Inspired by
[ImGuiTexInspect](https://github.com/ocornut/imgui/wiki/Useful-Extensions)
without vendoring C++.

**Package:** `texinspect` · **Modules:** `texinspect.zoom`, `texinspect.channels`,
`texinspect.inspect`, `texinspect.draw` · **0.1.0**  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Approach

| Path | Role |
|------|------|
| **Headless** (`inspect` / `zoom` / `channels`) | texture id, size, zoom milli, channel mask, pan — no window, no C++ |
| **Draw** (`texinspect.draw`) | ImGui window: id, zoom buttons, channel toggles, `ui.image` preview |

ImGuiTexInspect (C++) was **not** vendored: pure Ori meets G1 (show texture id;
zoom; channel toggle) without dual FULL `.a` (KISS). A future deepen may add
UV pick / histogram FULL client if needed.

## Build / smoke (Linux)

```bash
export ORI_BIN=/path/to/ori
export ORI_RUNTIME_CDYLIB=/path/to/libori_runtime.so
export ORI_RUNTIME_LIB=/path/to/libori_runtime.a
export ORI_USE_SYSTEM_LINKER=1
# smoke forces AOT: path-dep imgui is static-only (no libori_imgui.so for JIT)

./tools/build_linux.sh   # pure Ori marker under lib/<triple>/
./tools/smoke_linux.sh   # check + headless AOT run + ≥4 tests
```

Smoke **never** opens a window. Draw is only `ori check`'d when `../ori-imgui` exists.

## API

### `texinspect.inspect` (session)

| Symbol | Role |
|--------|------|
| `create(texture_id, width, height)` | Closed inspector bound to a texture id |
| `create_titled(...)` | Same with window title |
| `set_texture` / `texture_id_of` / `width_of` / `height_of` | Bind / query |
| `open_ui` / `close` / `is_open` | Visibility |
| `set_zoom_milli` / `zoom_in` / `zoom_out` / `reset_zoom` | Zoom |
| `display_width` / `display_height` | Scaled size from zoom |
| `toggle_channel_r/g/b/a` / `show_channel_*` / `set_channel_mask` | Channels |
| `set_pan_milli` / `reset_pan` | Pan offset (milli-px) |
| `bind_texture_for_test` | Headless rebind |

### `texinspect.zoom`

Milli factor: **1000 = 1.0x**. Clamped `[100, 16000]`.  
`create`, `set_milli`, `zoom_in`/`zoom_out`, `reset`, `scaled_size`.

### `texinspect.channels`

RGBA visibility (bool fields). Encoded mask 0..15 for interop (`R=1 G=2 B=4 A=8`).  
`toggle_*`, `enable_*`, `tint_*` (0/255 for draw).

### Draw (`texinspect.draw`)

```ori
import texinspect.inspect as insp
import texinspect.draw as draw
-- inside ImGui frame:
i = draw.draw(i)
```

## Phase OS

**Linux-first.** Windows/macOS packaging is **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (package).
