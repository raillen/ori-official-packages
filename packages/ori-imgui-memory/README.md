# ori-imgui-memory

In-ImGui **hex memory editor** for Ori (S3). Pure-Ori headless API to **bind a byte
buffer**, **read/write cells**, and format **hex rows**; product draw via sibling
[`ori-imgui`](../ori-imgui) (`imgui.ui`).

**Package:** `immemory` · **Modules:** `immemory.buffer`, `immemory.hex`,
`immemory.draw` · **0.1.0**  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Approach

| Path | Role |
|------|------|
| **Headless** (`immemory.buffer` + `immemory.hex`) | bind list/bytes, read/write cell, hex/ascii row lines — no window, no C++ |
| **Draw** (`immemory.draw`) | ImGui dump window + cell nudge buttons; path-dep `imgui` |

### Choice: pure Ori (not imgui_club C++)

[imgui_memory_editor](https://github.com/ocornut/imgui_club) was **not** vendored
for v0.1.0:

- G1 (bind buffer, read/write cell, smoke without GUI hang) is met with a pure
  Ori working copy of cells + hex format helpers.
- imgui_club is a C++ header client that needs ImGui draw lists / keyboard focus
  glue — high cost for Studio debug dumps that only need view/edit of a byte
  range in process memory.
- Aligns with plan guidance: *prefer pure Ori buffer view/edit over full
  imgui_club if G1 met; dual FULL only if C++*.

A future deepen may add a C++ FULL client (imgui_club) if live host-memory
pointer binding becomes a product requirement.

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

## API (`immemory.buffer`)

| Symbol | Role |
|--------|------|
| `create` / `create_with_size` | Empty or zero-filled buffer |
| `bind_list` / `bind_bytes` / `bind_for_test` | Bind working copy of cells (+ base addr) |
| `read_cell` / `write_cell` | Cell access (0–255 clamp; write no-op when RO/OOB) |
| `nbytes` / `is_empty` / `cells_of` / `to_bytes` | Size and export |
| `set_base_addr` / `base_addr_of` | Display base address |
| `set_cols` / `cols_of` | Bytes per row (1…`MAX_COLS`, default 16) |
| `set_cursor` / `cursor_of` | Selected offset (clamped) |
| `set_read_only` / `is_read_only` | Guard mutations |
| `resize` / `fill` / `clear_to_zero` | Mutate (blocked when RO) |
| `open_ui` / `close` / `is_open` / `set_title` | Window state for draw |

### Hex helpers (`immemory.hex`)

| Symbol | Role |
|--------|------|
| `byte_hex` | Two-digit uppercase hex for a byte |
| `address_hex` | Fixed-width uppercase address |
| `format_row_hex` / `format_row_ascii` / `format_row_line` | One dump row |
| `format_all_lines` | All rows as `list[string]` |
| `row_count` / `row_offset` / `row_nbytes` / `display_address` | Layout |

### Draw (`immemory.draw`)

```ori
import immemory.buffer as buf
import immemory.draw as draw
-- inside ImGui frame:
e = buf.open_ui(e)
e = draw.draw(e)
```

## Phase OS

**Linux-first.** Windows/macOS packaging is **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (package).
