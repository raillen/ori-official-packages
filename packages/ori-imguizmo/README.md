# ori-imguizmo

[ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) suite for Ori (S3): TRS gizmo, curve edit, gradient, zoom slider.

**Package:** `imguizmo` · **0.3.0**  
**Modules:** `imguizmo.gizmo` · `imguizmo.curve` · `imguizmo.gradient` · `imguizmo.zoom`  
**Maturity:** **5 (Linux)** — broad API + tests + smoke  
**Depends (product draw):** sibling `ori-imgui` ImGui context. Headless data APIs need no window.

## Build / smoke (Linux)

```bash
# Default dual path when ../ori-imgui is present:
#   libori_imguizmo_shim.{a,so}  — headless TRS + curve/gradient/zoom (smoke / ori test)
#   libori_imguizmo_full.a       — FULL ImGuizmo + ImCurveEdit + ImGradient (product AOT; link with ori-imgui)
./tools/build_linux.sh
./tools/smoke_linux.sh

# Headless-only (no FULL artifact):
ORI_IMGUIZMO_FULL=0 ./tools/build_linux.sh
```

Does **not** change the GLFW-only default of `ori-imgui` (no raylib embed required).

## Milli convention

`int` / `int64_t` ABI: float value × 1000.

| Domain | Unit |
|--------|------|
| Translation | world × 1000 |
| Rotation | degrees × 1000 |
| Scale | factor × 1000 (1000 = 1.0) |
| Curve t/v | domain units × 1000 |
| Gradient RGBA / t | 0..1 × 1000 |
| Zoom range/view/span | domain units × 1000 |

## API — `imguizmo.gizmo` (TRS, stable)

| Symbol | Role |
|--------|------|
| `identity()` | Reset TRS to identity; clear `last_op` |
| `set_translation_m` / `tx_m` `ty_m` `tz_m` / `translate_m` | Translation (milli) |
| `set_rotation_m` / `rx_m` `ry_m` `rz_m` / `rotate_m` | Euler degrees (milli) |
| `set_scale_m` / `sx_m` `sy_m` `sz_m` / `scale_m` | Uniform/non-uniform scale (milli) |
| `matrix_m(i)` / `set_matrix_m(i, v_m)` | Column-major 4×4 element 0..15 as milli-float |
| `last_op()` | `OP_NONE` / `OP_TRANSLATE` / `OP_ROTATE` / `OP_SCALE` |
| `manipulate_translate` / `manipulate_rotate` / `manipulate_scale` | FULL: ImGuizmo `Manipulate`; headless: false + bookkeep `last_op` |
| `set_view_m` / `set_proj_m` | View/projection matrix elements (FULL) |
| `is_full()` | True only when the **loaded** shim was built with `ORI_IMGUIZMO_FULL` |

## API — `imguizmo.curve` (ImCurveEdit)

| Symbol | Role |
|--------|------|
| `clear` / `point_count` | Reset / size |
| `add_point_m(t, v)` / `set_point_m(i, t, v)` | Edit points (sorted by t) |
| `point_t_m` / `point_v_m` | Accessors |
| `sample_m(t)` | Piecewise linear sample |
| `edit()` | FULL: `ImCurveEdit::Edit`; headless: false |

## API — `imguizmo.gradient` (ImGradient)

| Symbol | Role |
|--------|------|
| `clear` / `stop_count` | Reset / size |
| `add_stop_m(r,g,b,a,t)` / `set_stop_m(...)` | Color stops (sorted by t) |
| `stop_r_m` … `stop_t_m` | Accessors |
| `sample_r_m` … `sample_a_m` | Lerp sample at t |
| `edit()` | FULL: `ImGradient::Edit`; headless: false |

## API — `imguizmo.zoom` (ImZoomSlider)

| Symbol | Role |
|--------|------|
| `reset` / `set_range_m` / `set_view_m` | Bounds |
| `lower_m` / `higher_m` / `view_lower_m` / `view_higher_m` | Accessors |
| `span_m()` | **Zoom value** = view span (smaller ⇒ more zoomed in) |
| `slider()` | FULL: `ImZoomSlider`; headless: false |

### Product FULL link

Use `lib/x86_64-unknown-linux-gnu/libori_imguizmo_full.a` together with `ori-imgui` (active ImGui frame before `manipulate_*` / `edit` / `slider`). Smoke verifies the FULL archive exports TRS + curve/gradient/zoom symbols and ImGuizmo/ImCurveEdit/ImGradient.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity **5** / package **0.3.0**).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (ImGuizmo vendor license applies to `vendor/`).
