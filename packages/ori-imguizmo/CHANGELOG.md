# Changelog

## [0.3.0] - 2026-07-15

### Added
- **ImCurveEdit** module `imguizmo.curve`: point CRUD (milli), linear `sample_m`, FULL `edit()`
- **ImGradient** module `imguizmo.gradient`: color stops (RGBA+t milli), channel samples, FULL `edit()`
- **ImZoomSlider** module `imguizmo.zoom`: range/view bounds, `span_m` zoom value, FULL `slider()`
- Dual FULL builds ImCurveEdit.cpp + ImGradient.cpp with TRS Manipulate (ZoomSlider header-only)
- ≥6 new tests in `tests/test_curve_gradient_zoom.orl`; smoke covers curve/gradient/zoom

### Changed
- Version `0.2.0` → `0.3.0`
- Keep TRS APIs in `imguizmo.gizmo` stable

## [0.2.0] - 2026-07-15

### Added
- Maturity **5 (Linux)**: translate **and** rotate **and** scale (TRS) with milli ABI
- Matrix element get/set (`matrix_m` / `set_matrix_m`), `last_op`, view/proj setters
- FULL dual build: `libori_imguizmo_shim` (headless) + `libori_imguizmo_full.a` (ImGuizmo Manipulate)
- `manipulate_translate` / `manipulate_rotate` / `manipulate_scale`
- ≥5 automated tests + expanded smoke (TRS + headless manipulate)
- README API table + Phase OS note

### Changed
- Version `0.1.0` → `0.2.0`

## [0.1.0] - 2026-07-15

### Added
- ImGuizmo vendor + translation gizmo shim + tests
