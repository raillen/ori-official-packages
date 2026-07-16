# ori-imguidialog

In-ImGui **open/save file dialog** for Ori (S3). Pure-Ori state machine with
headless test hooks; product draw via sibling [`ori-imgui`](../ori-imgui)
(`imgui.ui`). Complements OS dialogs in [`ori-nfd`](../ori-nfd) and the lighter
list browser `imgui.file_browser`.

**Package:** `imguidialog` · **Modules:** `imguidialog.dialog`, `imguidialog.draw` · **0.1.0**  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Approach

| Path | Role |
|------|------|
| **Headless** (`imguidialog.dialog`) | open/save mode, filter, cancel, path result, `confirm_path_for_test` — no window, no C++ |
| **Draw** (`imguidialog.draw`) | ImGui window listing dirs/files; path-dep `imgui` |

ImGuiFileDialog (C++) was **not** vendored: pure Ori meets G1 without dual FULL `.a`
(KISS). A future deepen may add a C++ FULL client if preview/bookmarks are needed.

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

Smoke **never** opens a window. Draw is only `ori check`'d when `../ori-imgui` exists.

## API (`imguidialog.dialog`)

| Symbol | Role |
|--------|------|
| `create_open(title, start_dir, filter_ext)` | Open-file dialog (closed) |
| `create_save(title, start_dir, default_name, filter_ext)` | Save-as dialog |
| `open_ui(d)` | Show dialog; clear prior result |
| `confirm(d)` / `cancel(d)` | Accept dir+filename (filter-checked) / cancel |
| `set_filename` / `set_dir` / `set_filter` | Mutate selection |
| `matches_filter(name, filter_ext)` | `""` = all; multi `"orl;txt"` or `".orl,.txt"` |
| `is_open` / `is_confirmed` / `is_cancelled` / `result_path_of` | Query |
| `confirm_path_for_test(d, path)` | Headless inject full path (empty → cancel) |
| `cancel_for_test(d)` | Headless cancel |
| `take_result(d)` | Consumer finished reading path |

### Draw (`imguidialog.draw`)

```ori
import imguidialog.dialog = dlg
import imguidialog.draw = draw
-- inside ImGui frame:
d = draw.draw(d)
if dlg.is_confirmed(d)
    -- use dlg.result_path_of(d)
    d = dlg.take_result(d)
end
```

## Phase OS

**Linux-first.** Windows/macOS packaging is **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (package).
