# ori-imnodes

[imnodes](https://github.com/Nelarius/imnodes) node-graph editor for Ori (S3).

**Package:** `imnodes` · **Module:** `imnodes.editor` · **0.2.0**  
**Linux first** (Phase OS deferred) · maturity **5 (Linux)** · depends on `ori-imgui` when `ORI_IMNODES_FULL=1`

```bash
export ORI_BIN=…/ori ORI_USE_SYSTEM_LINKER=1 ORI_USE_JIT=1
./tools/smoke_linux.sh
```

## API

| Module | Symbol | Notes |
|--------|--------|-------|
| `imnodes.editor` | `create_context()` / `destroy_context()` | ImNodes context (no-op headless) |
| | `begin_node_editor()` / `end_node_editor()` | Editor canvas frame |
| | `begin_node(id)` / `end_node()` | Node body |
| | `begin_input_attr(id)` / `begin_output_attr(id)` / `end_attr()` | Pins (attributes) |
| | `link(id, start_attr, end_attr)` | Draw link between pins (FULL) |
| | `PIN_INPUT` / `PIN_OUTPUT` | Bookkeeping pin kinds (`0` / `1`) |
| | `graph_clear()` | Reset bookkeeping |
| | `graph_add_node(id) -> bool` | Register node id (idempotent) |
| | `graph_add_pin(id, kind) -> bool` | Register pin |
| | `graph_add_link(a, b) -> bool` | Link attrs (id = 0) |
| | `graph_add_link_id(link_id, a, b) -> bool` | Link with explicit id |
| | `graph_node_count()` / `graph_pin_count()` / `graph_link_count()` | Counts |
| | `graph_has_node(id) -> bool` | Membership |
| | `graph_pin_id(i)` / `graph_pin_kind(i)` | Pin query |
| | `graph_link_id(i)` / `graph_link_start(i)` / `graph_link_end(i)` | Link query |

## Build modes

| Mode | How | Behavior |
|------|-----|----------|
| **Headless (default)** | `./tools/build_linux.sh` | Graph bookkeeping + no-op editor APIs; CI/tests without ImGui frame |
| **FULL** | `ORI_IMNODES_FULL=1 ./tools/build_linux.sh` | Links real imnodes + imgui headers from sibling `ori-imgui` |

Headless tests never require mouse clicks: use `graph_*` for nodes/pins/links and counts.

## Layout

- `native/ori_imnodes_shim.cpp` — C++ shim (`int64_t` ABI)
- `vendor/` — imnodes sources
- `lib/x86_64-unknown-linux-gnu/` — staged `libori_imnodes_shim.{a,so}`
- `tests/`, `examples/smoke_imnodes.orl`

## Notes

- Caps: 64 nodes, 128 pins, 128 links (overflow returns `false` from `graph_add_*`).
- FULL draw path needs an active ImGui frame (same process as `ori-imgui`).

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy
