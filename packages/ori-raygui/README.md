# ori-raygui

[raygui](https://github.com/raysan5/raygui) bindings for [Ori](https://github.com/raillen/ori-lang) (S3).

**Package:** `raygui` · **Module:** `raygui.ui` · **Version:** 0.2.0  
**Requires:** Ori ≥ 0.3.0 · raylib (link `libraylib.a` + `libraygui.a`)

## Build native (Linux)

```bash
./tools/build_linux.sh
# expects vendor/raygui.h + vendor/raylib/*.h
# places lib/x86_64-unknown-linux-gnu/libraygui.a
# copy libraylib.a next to it (from ori-game setup) for demos
./tools/smoke_linux.sh
```

## Usage

Call widgets **inside** `BeginDrawing` / `EndDrawing` (same frame as raylib).

```ori
import raygui.ui = ui
-- …
if ui.button(20.0, 60.0, 120.0, 32.0, "OK")
    -- clicked
end
```

## Demo

```bash
ori compile examples/hello_raygui.orl -o /tmp/hello_raygui
/tmp/hello_raygui
```

## API

| Function | Role |
|----------|------|
| `button` | clickable |
| `label` | text |
| `checkbox` | bool toggle (check) |
| `toggle` | toggle button |
| `slider` | float range |
| `progress_bar` | float progress |
| `spinner` | int ± with optional edit mode |
| `value_box` | int value box + edit mode |
| `textbox` / `textbox_set` / `textbox_value` | single-line host buffer |
| `dropdown` | semicolon-separated items + edit mode |
| `list_view` | semicolon-separated list |
| `window_box` | closable window chrome (returns close click) |
| `panel` / `group_box` / `line` / `status_bar` | layout chrome |
| `set_style` / `get_style` | style property subset |
| `enable` / `disable` / `lock` / `unlock` | global gui state |

Style constants: `STYLE_DEFAULT`, `STYLE_BUTTON`, `STYLE_TEXTBOX`, … and
`PROP_TEXT_SIZE`, `PROP_BORDER_WIDTH`, color props, etc. (see `raygui/ui.orl`).
