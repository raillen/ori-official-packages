# ori-imgui-extras

ImGui **UX power tools** for Ori (S3): toast notify, list search, hotkey capture,
command palette, and frame metrics. Pure-Ori state machines with headless test
hooks; product UI composes sibling [`ori-imgui`](../ori-imgui) (`imgui.ui`).

**Package:** `imgui_extras` · **0.1.0**  
**Modules:** `imgui_extras.notify` · `.search` · `.hotkey` · `.command_palette` · `.metrics`  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Approach

| Module | Role |
|--------|------|
| **notify** | Toast queue with kind, TTL milli, max visible, `tick` expiry |
| **search** | Filterable string list (substring; optional case-sensitive) |
| **hotkey** | Capture/format chord strings (`Ctrl+Shift+S`); match live keys |
| **command_palette** | Register commands, filter by query, `run_by_id` / filtered index |
| **metrics** | Record frame milli; smoothed frame; `fps_milli` (fps×1000) |

Prefer **pure Ori** — no C++ host, no FULL `.a`. Path-dep `imgui` is for product
draw composition only; headless CI never opens a window.

## Build / smoke (Linux)

```bash
export ORI_BIN=/path/to/ori
export ORI_RUNTIME_CDYLIB=/path/to/libori_runtime.so
export ORI_RUNTIME_LIB=/path/to/libori_runtime.a
export ORI_USE_SYSTEM_LINKER=1
# smoke forces AOT: path-dep imgui is static-only (no libori_imgui.so for JIT)

./tools/build_linux.sh   # pure Ori marker under lib/<triple>/
./tools/smoke_linux.sh   # check + headless AOT run + ≥5 tests
```

## API sketch

### notify

```ori
import imgui_extras.notify = notify
var q = notify.create(3000, 5)
q = notify.push_info(q, "Saved")
q = notify.tick(q, frame_dt_milli)
```

### search

```ori
import imgui_extras.search = search
var fl = search.create()
fl = search.add_item(fl, "Player")
fl = search.set_query(fl, "play")
-- search.filtered(fl) / filtered_count / filtered_at
```

### hotkey

```ori
import imgui_extras.hotkey = hotkey
var h = hotkey.begin_capture(hotkey.create())
h = hotkey.feed_key(h, "S", true, true, false)
-- hotkey.chord_of(h) == "Ctrl+Shift+S"
```

### command_palette

```ori
import imgui_extras.command_palette = palette
var p = palette.create()
p = palette.register(p, "file.save", "Save", "Ctrl+S")
p = palette.open_ui(p)
p = palette.set_query(p, "save")
p = palette.run_filtered_at(p, 0)
-- palette.last_run_id_of(p) == "file.save"
```

### metrics

```ori
import imgui_extras.metrics = metrics
var m = metrics.create()
m = metrics.record_frame(m, 16)
-- metrics.frame_milli_of(m), metrics.fps_milli_of(m), metrics.integer_fps_of(m)
```

## Phase OS

**Linux-first.** Windows/macOS packaging is **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (package).
