# ori-implot3d

[ImPlot3D](https://github.com/brenocq/implot3d) for Ori (S3).

**Package:** `implot3d` · **Module:** `implot3d.charts` · **0.1.0**  
**Maturity:** **5 (Linux)** — broad API + tests + smoke  
**Depends (product draw):** sibling `ori-imgui` ImGui context. Headless series/labels need no window.  
**Path-dep (ecosystem):** sibling `ori-implot` (2D plots) + `ori-imgui` — ImPlot3D itself only needs ImGui headers.

## Build / smoke (Linux)

```bash
# Default dual path when ../ori-imgui is present:
#   libori_implot3d_shim.{a,so}  — headless (smoke / ori test)
#   libori_implot3d_full.a       — FULL ImPlot3D (product AOT; link with ori-imgui)
./tools/build_linux.sh
./tools/smoke_linux.sh

# Headless-only (no FULL artifact):
ORI_IMPLOT3D_FULL=0 ./tools/build_linux.sh
```

Does **not** change the GLFW-only default of `ori-imgui` (no raylib embed required).

## API (`implot3d.charts`)

| Symbol | Role |
|--------|------|
| `reset()` | Clear series + axis labels + draw state |
| `clear()` | Clear series / draw state (keeps axis labels) |
| `push_m(x_m, y_m, z_m) -> bool` | Append milli-int 3D sample (false at capacity) |
| `count()` / `capacity()` | Series size |
| `sum_m()` / `min_m()` / `max_m()` | Stats on **Z** (milli) |
| `setup_axes(x, y, z)` | Store axis labels (applied on FULL begin) |
| `x/y/z_label_eq` / `*_len` | Label query helpers |
| `begin_plot(title, w, h) -> bool` | FULL: ImPlot3D begin; headless: false |
| `plot_line` / `plot_scatter` | Draw series (FULL) or bookkeeping |
| `end_plot()` | End plot frame |
| `last_draw()` | `DRAW_NONE` / `LINE` / `SCATTER` |
| `create_context` / `destroy_context` | ImPlot3D context (FULL) |
| `is_full()` | True only when the **loaded** shim was built with `ORI_IMPLOT3D_FULL` |

### Milli convention

Samples are **milli-units** (`int`): `1000` → `1.0` double in the native buffer. Same as `ori-implot`.

### Product FULL link

Use `lib/x86_64-unknown-linux-gnu/libori_implot3d_full.a` together with `ori-imgui` (active ImGui frame before `begin_plot`). Smoke verifies the FULL archive is produced and exports line/scatter + ImPlot3D symbols.

Vendor: `brenocq/implot3d` (MIT) — see `vendor/VERSION.txt`.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (ImPlot3D vendor license applies to `vendor/`).
