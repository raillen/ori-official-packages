# ori-imgui-textedit

In-ImGui **multiline text / code buffer** for Ori (S3). Pure-Ori headless API with
a **language highlight stub**; product draw via sibling [`ori-imgui`](../ori-imgui)
(`imgui.ui` multiline + colored token preview).

**Package:** `imtextedit` · **Modules:** `imtextedit.buffer`, `imtextedit.lang`,
`imtextedit.draw` · **0.1.0**  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Approach

| Path | Role |
|------|------|
| **Headless** (`imtextedit.buffer` + `imtextedit.lang`) | set/get text, cursor, lines, read-only, language id, token spans — no window, no C++ |
| **Draw** (`imtextedit.draw`) | ImGui window + multiline host slot + highlight preview; path-dep `imgui` |

### Choice: pure Ori (not pthom ColorTextEdit C++)

[ImGuiColorTextEdit (pthom)](https://github.com/pthom/ImGuiColorTextEdit/tree/imgui_bundle)
was **not** vendored for v0.1.0:

- G1 (set/get text, language stub, headless buffer + FULL draw when linked) is met
  without a large C++ dependency / dual FULL `.a`.
- ColorTextEdit needs ImGui C++ linkage, regex/grammar tables, and heavy host glue —
  high cost vs Studio use of an **external IDE** (VS Code + ori-lsp) for real editing.
- Aligns with plan guidance: *prefer pure Ori for UX widgets when C++ not required*
  and PR brief: *pure-Ori multiline buffer if C++ too heavy — document choice*.

A future deepen may add a C++ FULL client (pthom fork) if in-UI full syntax editing
becomes a product requirement.

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

## API (`imtextedit.buffer`)

| Symbol | Role |
|--------|------|
| `create` / `create_with_text` | New empty or seeded editor |
| `set_text` / `text_of` / `text_nbytes` / `is_empty` / `clear` | Buffer body |
| `set_language` / `language_of` | `LANG_NONE`, `LANG_PLAIN`, `LANG_ORI`, `LANG_C` |
| `set_cursor` / `cursor_of` | Cursor offset (clamped) |
| `append` / `insert_at_cursor` | Mutate (no-op when read-only) |
| `line_count` / `line_at` / `lines_of` | Line model (`\n`) |
| `set_read_only` / `is_read_only` | Guard mutations |

### Language stub (`imtextedit.lang`)

| Symbol | Role |
|--------|------|
| `highlight_spans` / `highlight_text` | Token spans for current language |
| `language_name` / `token_kind_name` | Human labels |
| `count_kind` / `span_slice` | Headless test helpers |
| Kinds | `KIND_PLAIN`, `KEYWORD`, `COMMENT`, `STRING`, `NUMBER`, `IDENT` |

Stub only: `//` comments, `"…"` strings, digits, identifiers vs small keyword tables
for Ori and C. Not a full grammar / regex engine.

### Draw (`imtextedit.draw`)

```ori
import imtextedit.buffer = buf
import imtextedit.draw = draw
-- inside ImGui frame:
e = draw.draw(e, "Script")
-- e.text updated from host multiline when edited
```

Host input slot: `draw.HOST_SLOT` (default **8**). Also `seed_host_slot` / `pull_host_slot`.

## Phase OS

**Linux-first.** Windows/macOS packaging is **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (package).
