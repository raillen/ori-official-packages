# ori-clay

[Clay](https://github.com/nicbarker/clay) immediate-mode UI layout for Ori (S3).

**Package:** `clay` · **Module:** `clay.layout` · **0.2.0**  
**Linux first** · maturity **5 (Linux)** · **headless** (no window / raylib)

```bash
./tools/smoke_linux.sh
```

## API

| API | Description |
|-----|-------------|
| `init(w, h)` | Create Clay arena for layout size (pixels) |
| `layout_two_boxes()` | Root + 2 fixed LTR siblings (`pad=16`, `gap=16`); returns 2 |
| `layout_nested()` | Nested multi-box (see layout tree); returns 4 |
| `box_x/y/w/h_m(i)` | Bounds of box `i` in **milli**-pixels (`value * 1000`) |
| `box_count()` | Boxes stored after last layout |
| `cmd_count()` / `rect_cmd_count()` | Render commands / rectangle-only count |
| `cmd_type(i)` | `CMD_NONE=0`, `CMD_RECTANGLE=1`, `CMD_BORDER=2`, `CMD_TEXT=3` |
| `cmd_x/y/w/h_m(i)` | Command bounding box in milli-pixels |
| `cmd_r/g/b/a(i)` | Rectangle fill color channels (0–255) |
| `shutdown()` | Free arena |

### Nested layout tree (`layout_nested`)

```
Root   grow, pad=20, gap=10, TOP_TO_BOTTOM
├── Header  fixed 200×40          → box index BOX_HEADER (0)
└── Body    grow w, h=100, pad=12, gap=8, LEFT_TO_RIGHT  → BOX_BODY (1)
    ├── Left   fixed 80×50        → BOX_LEFT (2)
    └── Right  fixed 90×50        → BOX_RIGHT (3)
```

Expected milli bounds (800×600 canvas): Header `(20k,20k)`, Body `(20k,70k)`, Left `(32k,82k)`, Right `(120k,82k)`.

### Command list export (drawers)

After any layout call, Clay’s render command array is copied into a fixed buffer (max 64). A pure-Ori or raylib drawer can iterate:

```text
for i in 0..cmd_count()-1:
  if cmd_type(i) == CMD_RECTANGLE:
    draw_rect(cmd_x_m(i)/1000, …, cmd_r(i), cmd_g(i), cmd_b(i), cmd_a(i))
```

Nested layout yields **5** rectangles (Root, Header, Body, Left, Right).

### Two-box pad/gap

Root `pad=16`, `childGap=16`, LTR → BoxA `(16,16)` size 100×50; BoxB x = `16+100+16` = 132.

## Constants

| Constant | Value |
|----------|-------|
| `BOX_HEADER` / `BOX_BODY` / `BOX_LEFT` / `BOX_RIGHT` | 0…3 |
| `CMD_RECTANGLE` | 1 |

## Milli-float ABI

All geometry crosses the FFI as `int64_t` milli-units (`pixels * 1000`). Colors are integer 0–255.

Upstream `clay.h` is zlib-licensed (see `vendor/LICENSE.clay`).

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See [`ori-lang` `docs/planning/PHASE-OS.md`](https://github.com/raillen/ori-lang/blob/master/docs/planning/PHASE-OS.md) for the multi-OS staging policy
