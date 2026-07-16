# Changelog

## [0.1.0] - 2026-07-15

### Added
- Package `markdown` **0.1.0** — pure-Ori in-ImGui markdown subset (G1–G7)
- `markdown.parse`: headings (`#`…`######`), paragraphs, fenced code (` ```lang `) → `Document` / `Block`
- `markdown.draw`: product ImGui window + inline draw (path-dep `ori-imgui`)
- `markdown.ime_stub`: Linux no-op IME surface; real IMM32 documented as Phase OS Windows-only
- ≥5 unit tests + `examples/smoke_markdown.orl` + `tools/smoke_linux.sh`
- Documented choice: pure Ori over C++ imgui_markdown for v0.1.0
- Phase OS deferred note (`tools/build_windows.ps1`)
- Maturity **5 (Linux)** claim
