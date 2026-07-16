# ori-freetype

[FreeType](https://freetype.org/) bindings for [Ori](https://github.com/raillen/ori-lang) (S3).

**Package:** `freetype` · **Modules:** `freetype.face`, `freetype.text` · **Version:** 0.1.0  
**Requires:** Ori ≥ 0.3.0 · `libfreetype` (dev package) · Linux first

## Build / smoke (Linux)

```bash
# Debian/Ubuntu: sudo apt install libfreetype6-dev
./tools/build_linux.sh
ORI_USE_SYSTEM_LINKER=1 ./tools/smoke_linux.sh
```

Optional font override:

```bash
export ORI_FT_SMOKE_FONT=/path/to/font.ttf
```

## API

### `freetype.face`

| Function | Role |
|----------|------|
| `init` / `done` | FreeType library lifecycle |
| `new_face` / `new_face_index` / `done_face` | Load TTF/OTF by path |
| `set_pixel_sizes` | Pixel height (width 0 = proportional) |
| `render_char` / `render_glyph_index` | Rasterize one glyph |
| `char_index` | Unicode → glyph index |
| `bitmap_*` / `bearing_*` / `advance_m` / `bitmap_sample` | Last glyph metrics |
| `write_bitmap_pgm` | Debug dump of last bitmap |
| `ascender_m` / `descender_m` / `height_m` | Face metrics (milli-pixel) |
| `has_kerning` / `kerning_x_m` | Pair kerning |
| `units_per_em` / `num_faces` / `num_glyphs` | Face metadata |
| `face_ptr` / `is_valid_face` | Handle validation + FT_Face* for HarfBuzz |

### `freetype.text`

| Function | Role |
|----------|------|
| `measure_char` | One codepoint → `GlyphMetrics` |
| `face_metrics` / `line_height_m` | Face-level metrics |
| `measure_codepoints` / `measure_codepoints_advance_m` | LTR list + optional kerning |

Complex scripts / full strings → **`ori-harfbuzz`**.

## Dependency

```toml
[dependencies]
freetype = { path = "../ori-freetype", version = "0.1.0" }
```
