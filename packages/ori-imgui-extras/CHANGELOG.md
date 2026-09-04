# Changelog

## [0.1.0] - 2026-07-15

### Added
- Package `imgui_extras` **0.1.0** — pure-Ori ImGui UX extras (G1–G7)
- `imgui_extras.notify`: toast queue (`push_info`/`warn`/`error`, `tick`, cap, TTL)
- `imgui_extras.search`: filterable list (`set_query`, case option, `filtered*`)
- `imgui_extras.hotkey`: capture chord string (`feed_key`, `format_chord`, match)
- `imgui_extras.command_palette`: register/filter/`run_by_id`/`run_filtered_at`
- `imgui_extras.metrics`: frame milli + `fps_milli` / integer FPS
- ≥5 unit tests + `examples/smoke_imgui_extras.orl` + `tools/smoke_linux.sh`
- Path-dep sibling `ori-imgui` 0.4.1 (product host; headless needs no draw)
- Phase OS deferred note (`tools/build_windows.ps1`)
- Maturity **5 (Linux)** claim
