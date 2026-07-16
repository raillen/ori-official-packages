# ori-imgui-markdown

In-ImGui **markdown subset** for Ori (S3). Pure-Ori headless API to **parse
headings, paragraphs, and fenced code** into structures; product draw via sibling
[`ori-imgui`](../ori-imgui) (`imgui.ui`). Ships **IME Linux no-op stubs** with a
clear policy that real IMM32 is **Phase OS / Windows-only**.

**Package:** `markdown` · **Modules:** `markdown.parse`, `markdown.draw`,
`markdown.ime_stub` · **0.1.0**  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Approach

| Path | Role |
|------|------|
| **Headless** (`markdown.parse`) | G1 parse → `Document` / `Block` — no window, no C++ |
| **Draw** (`markdown.draw`) | ImGui dump window + inline blocks; path-dep `imgui` |
| **IME** (`markdown.ime_stub`) | Linux no-op stubs; `supports_real_ime` always false |

### Choice: pure Ori (not imgui_markdown C++)

[imgui_markdown](https://github.com/ocornut/imgui/wiki/Useful-Extensions) was
**not** vendored for v0.1.0:

- G1 (headings + paragraphs + fenced code → ImGui text) is met with a pure Ori
  line scanner and optional `imgui.ui` text/colored draw.
- C++ imgui_markdown needs ImGui draw-list / link / image callbacks — high cost
  for Studio help panels that only need structured text dump.
- Aligns with plan guidance: *prefer pure Ori markdown subset when G1 met*.

A future deepen may add a C++ FULL client if link callbacks / image embeds become
product requirements.

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

## API (`markdown.parse`)

| Symbol | Role |
|--------|------|
| `parse` / `parse_with_title` | Source → `Document` with `Block` list |
| `KIND_HEADING` / `KIND_PARAGRAPH` / `KIND_CODE_FENCE` | Block kinds |
| `heading_block` / `paragraph_block` / `code_fence_block` | Manual constructors |
| `block_count` / `block_at` / `count_kind` | Inspect blocks |
| `first_heading_text` / `source_of` / `kind_name` | Queries |
| `open_ui` / `close` / `is_open` / `set_title` | Window state for draw |

### Draw (`markdown.draw`)

```ori
import markdown.parse = md
import markdown.draw = draw
-- inside ImGui frame:
doc = md.open_ui(doc)
doc = draw.draw(doc)
-- or embed without chrome:
draw.draw_inline(doc)
```

### IME policy (`markdown.ime_stub`)

| Symbol | Linux stub behavior |
|--------|---------------------|
| `create` / `create_for_platform` | State with `PLATFORM_LINUX` (or labeled platform) |
| `enable` / `disable` / `is_enabled` | Flag only — no OS connection |
| `set_composition` / `begin_composition` | **No-op** — composition stays empty |
| `composition_text` / `is_composing` | Always empty / false after stub ops |
| `process_key` | Always `false` (never consumes) |
| `supports_real_ime` | Always `false` |
| `is_windows_only_real_impl` | Always `true` |

**Real IMM32 (CJK composition, candidate list, Win32 IME messages)** is
**Windows-only** and deferred to **Phase OS**. See ori-lang
`docs/planning/PHASE-OS.md`. Do not expect fcitx/ibus/IMM32 wiring from this
package on Linux.

## Phase OS

**Linux-first.** Windows/macOS packaging and real IME are **deferred** (Phase OS
last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1)
- IME real path: Windows IMM32 only (not implemented in 0.1.0)

## License

MIT (package).
