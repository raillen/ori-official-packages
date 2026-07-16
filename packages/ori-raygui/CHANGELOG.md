# Changelog

## [Unreleased]

### Added
- Integration demo under `demos/` (with ori-game path dep).

## [0.2.0] - 2026-07-14

### Added
- `textbox` / `textbox_set` / `textbox_value` (host buffer)
- `toggle`, `progress_bar`, `spinner`, `value_box`
- `window_box`, `panel`, `line`
- `dropdown` / `list_view` (semicolon-separated items)
- `set_style` / `get_style` + style property constants subset
- Expanded `examples/hello_raygui.orl`

## [0.1.0] - 2026-07-13

### Added
- MVP: button, label, checkbox, slider, group_box, status_bar

## [Unreleased]

### Added
- Integration demo `demos/raygui_game` (raygui + game.app).
- Renamed conflicting root `raylib.orl` → `examples/raylib_raw.orl` (module `raylib_raw`) to allow path-dep with ori-game.
