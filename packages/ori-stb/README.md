# ori-stb

[stb](https://github.com/nothings/stb) single-header bindings for Ori (S3).

**Package:** `stb` · **Modules:** `stb.image`, `stb.perlin`, `stb.rect_pack` · **0.2.0**  
**Linux first** (Phase OS deferred) · maturity **5 (Linux)** gate: broad API + tests + smoke

```bash
export ORI_BIN=…/ori ORI_USE_SYSTEM_LINKER=1 ORI_USE_JIT=1
./tools/smoke_linux.sh
```

## API

| Module | Symbol | Notes |
|--------|--------|-------|
| `stb.image` | `load(path, desired_channels) -> bool` | Path load; 0 channels = source layout |
| | `width()` / `height()` / `channels()` | Dims after successful load |
| | `pixels_ptr()` / `pixel_sum()` | Raw buffer pointer (int64 ABI) / diagnostic sum |
| | `free()` | Release last load |
| | `write_png_gray(path, w, h, data_ptr)` | Write 8-bit gray from caller buffer |
| | `write_solid_gray_png(path, w, h, gray)` | Convenience solid gray PNG |
| `stb.perlin` | `noise2_m(x, y) -> int` | 2D Perlin, milli in/out (~-1000..1000) |
| | `noise3_m(x, y, z) -> int` | 3D Perlin, milli in/out |
| `stb.rect_pack` | `begin_pack(count) -> bool` | Init session (max 64 rects) |
| | `add_rect(w, h) -> bool` | Queue rect |
| | `run(atlas_w, atlas_h) -> int` | Pack; returns packed count |
| | `rect_x/y/w/h(i)` / `was_packed(i)` / `rect_count()` | Query packed layout |

## Layout

- `native/ori_stb_shim.c` — C shim (`int64_t` ABI)
- `vendor/` — stb headers (`stb_image`, `stb_image_write`, `stb_perlin`, `stb_rect_pack`)
- `lib/x86_64-unknown-linux-gnu/` — staged `libori_stb_shim.{a,so}`
- `tests/`, `examples/smoke_stb.orl`

## Notes

- Noise uses **milli-float** (value × 1000 as `int`).
- Image load keeps one global slot (last load); call `free` between independent images.
- `stb_vorbis` is vendored but not exposed yet.

## Phase OS

**Linux-first.** Windows/macOS native builds are **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1) (echoes deferred; does not produce MSVC libs)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy
