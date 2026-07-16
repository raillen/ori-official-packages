# Changelog

## [0.1.0] - 2026-07-15

### Added
- Package `texinspect` **0.1.0** — pure-Ori texture inspect MVP (G1–G7)
- `texinspect.inspect`: bind texture id + size, open/close, pan milli
- `texinspect.zoom`: milli zoom (1000=1x), clamp, scaled display size
- `texinspect.channels`: RGBA toggle + mask 0..15 encode/decode + tint 0/255
- `texinspect.draw`: product ImGui window via path-dep `ori-imgui` (`imgui.ui`)
- ≥4 unit tests + `examples/smoke_texinspect.orl` + `tools/smoke_linux.sh`
- Phase OS deferred note (`tools/build_windows.ps1`)
- Maturity **5 (Linux)** claim
