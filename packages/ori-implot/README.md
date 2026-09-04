# ori-implot

[ImPlot](https://github.com/epezent/implot) for Ori (S3).

**Package:** `implot` · **Module:** `implot.charts` · **0.2.0**  
**Maturity:** **5 (Linux)** — broad API + tests + smoke  
**Depends (product draw):** sibling `ori-imgui` ImGui context. Headless series/labels need no window.

## Build / smoke (Linux)

```bash
# Default dual path when ../ori-imgui is present:
#   libori_implot_shim.{a,so}  — headless (smoke / ori test)
#   libori_implot_full.a       — FULL ImPlot (product AOT; link with ori-imgui)
./tools/build_linux.sh
./tools/smoke_linux.sh

# Headless-only (no FULL artifact):
ORI_IMPLOT_FULL=0 ./tools/build_linux.sh
```

Does **not** change the GLFW-only default of `ori-imgui` (no raylib embed required).

## API (`implot.charts`)

| Symbol | Role |
|--------|------|
| `reset()` | Clear series + axis labels + draw state |
| `clear()` | Clear series / draw state (keeps axis labels) |
| `push_m(x_m, y_m) -> bool` | Append milli-int sample (false at capacity) |
| `count()` / `capacity()` | Series size |
| `sum_m()` / `min_m()` / `max_m()` | Stats on Y (milli) |
| `setup_axes(x_label, y_label)` | Store axis labels (applied on FULL begin) |
| `x_label_eq` / `y_label_eq` / `*_len` | Label query helpers |
| `begin_plot(title, w, h) -> bool` | FULL: ImPlot begin; headless: false |
| `plot_line` / `plot_scatter` / `plot_bars` | Draw series (FULL) or bookkeeping |
| `end_plot()` | End plot frame |
| `last_draw()` | `DRAW_NONE` / `LINE` / `SCATTER` / `BARS` |
| `create_context` / `destroy_context` | ImPlot context (FULL) |
| `is_full()` | True only when the **loaded** shim was built with `ORI_IMPLOT_FULL` |

### Product FULL link

Use `lib/x86_64-unknown-linux-gnu/libori_implot_full.a` together with `ori-imgui` (active ImGui frame before `begin_plot`). Smoke verifies the FULL archive is produced and exports scatter/bars.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (ImPlot vendor license applies to `vendor/`).
