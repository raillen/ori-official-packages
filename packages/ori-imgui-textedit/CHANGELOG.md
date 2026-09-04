# Changelog

## [0.1.0] - 2026-07-15

### Added
- Package `imtextedit` **0.1.0** — pure-Ori multiline text editor buffer (G1–G7)
- `imtextedit.buffer`: set/get text, cursor, lines, append/insert, read-only, language id
- `imtextedit.lang`: language stub (`none`/`plain`/`ori`/`c`) + highlight token spans
- `imtextedit.draw`: product ImGui multiline + colored highlight preview (path-dep `ori-imgui`)
- ≥5 unit tests + `examples/smoke_imtextedit.orl` + `tools/smoke_linux.sh`
- Documented choice: pure Ori over pthom ColorTextEdit C++ for v0.1.0
- Phase OS deferred note (`tools/build_windows.ps1`)
- Maturity **5 (Linux)** claim
