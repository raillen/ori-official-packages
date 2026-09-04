# ori-harfbuzz

[HarfBuzz](https://harfbuzz.github.io/) text shaping for [Ori](https://github.com/raillen/ori-lang) (S3).

**Package:** `harfbuzz` · **Module:** `harfbuzz.shape` · **Version:** 0.1.0  
**Depends on:** sibling **`ori-freetype`** (face + pixel size)  
**Requires:** Ori ≥ 0.3.0 · `libharfbuzz-dev` · `libfreetype6-dev` · Linux first

## Build / smoke (Linux)

```bash
# Debian/Ubuntu: sudo apt install libharfbuzz-dev libfreetype6-dev
# Sibling checkout:
#   Documentos/Projetos/ori-freetype
#   Documentos/Projetos/ori-harfbuzz
./tools/smoke_linux.sh
```

Validation is **JIT-first** (`ori run` smoke). AOT `ori test` runs only when static
`libharfbuzz.a` is staged (many distros ship shared-only HarfBuzz).

## API

| Function | Role |
|----------|------|
| `shape(face, text)` | UTF-8 shape, auto direction/script → `list[ShapedGlyph]` |
| `shape_dir(face, text, dir)` | Explicit `DIR_LTR` / `RTL` / `TTB` / `BTT` / `AUTO` |
| `glyph_count` | length |
| `total_x_advance_m` / `total_y_advance_m` | Sum of advances (milli-pixel) |
| `x_cursor_positions_m` | Cursor x after each glyph (length = n+1) |

`ShapedGlyph`: `glyph_id`, `cluster`, `x_advance_m`, `y_advance_m`, `x_offset_m`, `y_offset_m`.

Uses FreeType face pointer via `freetype.face.face_ptr` + `hb_ft_font_create_referenced`.

## Dependency

```toml
[dependencies]
freetype = { path = "../ori-freetype", version = "0.1.0" }
harfbuzz = { path = "../ori-harfbuzz", version = "0.1.0" }
```
