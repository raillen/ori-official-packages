# Changelog

## [0.1.0] - 2026-07-15

### Added
- Package `imguidialog` **0.1.0** — pure-Ori in-ImGui open/save dialog (G1–G7)
- `imguidialog.dialog`: `create_open` / `create_save`, `open_ui`, `confirm` / `cancel`
- Filter API: `matches_filter` (single + multi `orl;txt` / `.orl,.txt`), confirm rejects mismatch
- Path result queries: `result_path_of`, `is_confirmed`, `is_cancelled`, `take_result`
- Headless hooks: `confirm_path_for_test`, `cancel_for_test` (CI without GUI)
- `imguidialog.draw`: product ImGui listing via path-dep `ori-imgui` (`imgui.ui`)
- ≥5 unit tests + `examples/smoke_imguidialog.orl` + `tools/smoke_linux.sh`
- Phase OS deferred note (`tools/build_windows.ps1`)
- Maturity **5 (Linux)** claim
