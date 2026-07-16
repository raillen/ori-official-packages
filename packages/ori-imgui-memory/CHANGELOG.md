# Changelog

## [0.1.0] - 2026-07-15

### Added
- Package `immemory` **0.1.0** — pure-Ori hex memory editor (G1–G7)
- `immemory.buffer`: bind list/bytes, read/write cell, cols, base addr, cursor, RO, resize/fill
- `immemory.hex`: byte/address hex, row hex/ascii/line helpers, `format_all_lines`
- `immemory.draw`: product ImGui dump window (path-dep `ori-imgui`)
- ≥5 unit tests + `examples/smoke_immemory.orl` + `tools/smoke_linux.sh`
- Documented choice: pure Ori over imgui_club C++ for v0.1.0
- Phase OS deferred note (`tools/build_windows.ps1`)
- Maturity **5 (Linux)** claim
