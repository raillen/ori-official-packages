/* FreeType shim for Ori — int handles; pointer args as int64 (Ori int is 64-bit). */
#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_FACES 32
#define BITMAP_CAP (512 * 512)

static FT_Library g_lib = NULL;
static FT_Face g_faces[MAX_FACES];

static unsigned char g_bitmap[BITMAP_CAP];
static int g_bmp_w = 0;
static int g_bmp_h = 0;
static int g_bmp_pitch = 0;
static int g_bearing_x = 0;
static int g_bearing_y = 0;
static int g_advance_m = 0;
static char g_err[256];

static void set_err(const char *msg) {
    if (!msg) {
        g_err[0] = '\0';
        return;
    }
    strncpy(g_err, msg, sizeof(g_err) - 1);
    g_err[sizeof(g_err) - 1] = '\0';
}

int64_t ori_ft_init(void) {
    if (g_lib != NULL) {
        return 0;
    }
    if (FT_Init_FreeType(&g_lib) != 0) {
        set_err("FT_Init_FreeType failed");
        return 1;
    }
    set_err(NULL);
    return 0;
}

void ori_ft_done(void) {
    for (int i = 0; i < MAX_FACES; ++i) {
        if (g_faces[i]) {
            FT_Done_Face(g_faces[i]);
            g_faces[i] = NULL;
        }
    }
    if (g_lib) {
        FT_Done_FreeType(g_lib);
        g_lib = NULL;
    }
}

int64_t ori_ft_new_face(int64_t path_ptr, int64_t face_index) {
    if (!g_lib) {
        if (ori_ft_init() != 0) {
            return -1;
        }
    }
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        set_err("null path");
        return -1;
    }
    int slot = -1;
    for (int i = 0; i < MAX_FACES; ++i) {
        if (g_faces[i] == NULL) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        set_err("too many faces");
        return -1;
    }
    FT_Face face = NULL;
    if (FT_New_Face(g_lib, path, (FT_Long)face_index, &face) != 0) {
        set_err("FT_New_Face failed");
        return -1;
    }
    g_faces[slot] = face;
    set_err(NULL);
    return (int64_t)slot;
}

void ori_ft_done_face(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) {
        return;
    }
    FT_Done_Face(g_faces[face]);
    g_faces[face] = NULL;
}

int64_t ori_ft_set_pixel_sizes(int64_t face, int64_t width, int64_t height) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) {
        set_err("invalid face");
        return 1;
    }
    FT_Error err = FT_Set_Pixel_Sizes(g_faces[face], (FT_UInt)width, (FT_UInt)height);
    if (err != 0) {
        FT_F26Dot6 h = (FT_F26Dot6)(height <= 0 ? 16 : height) * 64;
        FT_F26Dot6 w = (FT_F26Dot6)(width <= 0 ? 0 : width) * 64;
        err = FT_Set_Char_Size(g_faces[face], w, h, 72, 72);
    }
    if (err != 0) {
        set_err("FT_Set_Pixel_Sizes failed");
        return 1;
    }
    set_err(NULL);
    return 0;
}

int64_t ori_ft_render_char(int64_t face, int64_t codepoint) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) {
        set_err("invalid face");
        return 1;
    }
    FT_Face f = g_faces[face];
    FT_UInt gi = FT_Get_Char_Index(f, (FT_ULong)codepoint);
    if (FT_Load_Glyph(f, gi, FT_LOAD_DEFAULT) != 0) {
        set_err("FT_Load_Glyph failed");
        return 1;
    }
    if (FT_Render_Glyph(f->glyph, FT_RENDER_MODE_NORMAL) != 0) {
        set_err("FT_Render_Glyph failed");
        return 1;
    }
    FT_Bitmap *bm = &f->glyph->bitmap;
    g_bmp_w = (int)bm->width;
    g_bmp_h = (int)bm->rows;
    g_bmp_pitch = (int)bm->pitch;
    g_bearing_x = (int)f->glyph->bitmap_left;
    g_bearing_y = (int)f->glyph->bitmap_top;
    g_advance_m = (int)((f->glyph->advance.x * 1000) / 64);
    int need = g_bmp_h * (g_bmp_pitch > 0 ? g_bmp_pitch : g_bmp_w);
    if (need < 0) {
        need = 0;
    }
    if (need > BITMAP_CAP) {
        set_err("glyph bitmap too large");
        return 1;
    }
    if (bm->buffer && need > 0) {
        memcpy(g_bitmap, bm->buffer, (size_t)need);
    }
    set_err(NULL);
    return 0;
}

int64_t ori_ft_bitmap_width(void) { return g_bmp_w; }
int64_t ori_ft_bitmap_height(void) { return g_bmp_h; }
int64_t ori_ft_bitmap_pitch(void) { return g_bmp_pitch; }
int64_t ori_ft_bearing_x(void) { return g_bearing_x; }
int64_t ori_ft_bearing_y(void) { return g_bearing_y; }
int64_t ori_ft_advance_m(void) { return g_advance_m; }

int64_t ori_ft_bitmap_ink_sum(void) {
    int pitch = g_bmp_pitch > 0 ? g_bmp_pitch : g_bmp_w;
    int64_t sum = 0;
    for (int y = 0; y < g_bmp_h; ++y) {
        for (int x = 0; x < g_bmp_w; ++x) {
            sum += (int64_t)g_bitmap[y * pitch + x];
        }
    }
    return sum;
}

/* Borrowed FT_Face* for HarfBuzz (same process). Returns 0 if invalid. */
int64_t ori_ft_face_ptr(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) {
        return 0;
    }
    return (int64_t)(intptr_t)g_faces[face];
}

/* Write last glyph grayscale bitmap as PGM (smoke / debug). path as int64 ptr. */
int64_t ori_ft_write_bitmap_pgm(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path || g_bmp_w <= 0 || g_bmp_h <= 0) {
        set_err("no bitmap");
        return 1;
    }
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        set_err("fopen failed");
        return 1;
    }
    fprintf(fp, "P5\n%d %d\n255\n", g_bmp_w, g_bmp_h);
    int pitch = g_bmp_pitch > 0 ? g_bmp_pitch : g_bmp_w;
    for (int y = 0; y < g_bmp_h; ++y) {
        fwrite(g_bitmap + y * pitch, 1, (size_t)g_bmp_w, fp);
    }
    fclose(fp);
    set_err(NULL);
    return 0;
}

int64_t ori_ft_render_glyph_index(int64_t face, int64_t glyph_index) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) {
        set_err("invalid face");
        return 1;
    }
    FT_Face f = g_faces[face];
    if (FT_Load_Glyph(f, (FT_UInt)glyph_index, FT_LOAD_DEFAULT) != 0) {
        set_err("FT_Load_Glyph failed");
        return 1;
    }
    if (FT_Render_Glyph(f->glyph, FT_RENDER_MODE_NORMAL) != 0) {
        set_err("FT_Render_Glyph failed");
        return 1;
    }
    FT_Bitmap *bm = &f->glyph->bitmap;
    g_bmp_w = (int)bm->width;
    g_bmp_h = (int)bm->rows;
    g_bmp_pitch = (int)bm->pitch;
    g_bearing_x = (int)f->glyph->bitmap_left;
    g_bearing_y = (int)f->glyph->bitmap_top;
    g_advance_m = (int)((f->glyph->advance.x * 1000) / 64);
    int need = g_bmp_h * (g_bmp_pitch > 0 ? g_bmp_pitch : g_bmp_w);
    if (need < 0) need = 0;
    if (need > BITMAP_CAP) {
        set_err("glyph bitmap too large");
        return 1;
    }
    if (bm->buffer && need > 0) {
        memcpy(g_bitmap, bm->buffer, (size_t)need);
    }
    set_err(NULL);
    return 0;
}

int64_t ori_ft_char_index(int64_t face, int64_t codepoint) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) {
        return 0;
    }
    return (int64_t)FT_Get_Char_Index(g_faces[face], (FT_ULong)codepoint);
}

/* Face metrics in milli-pixel at current size (ascender/descender/height). */
int64_t ori_ft_ascender_m(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    return (int64_t)((g_faces[face]->size->metrics.ascender * 1000) / 64);
}

int64_t ori_ft_descender_m(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    return (int64_t)((g_faces[face]->size->metrics.descender * 1000) / 64);
}

int64_t ori_ft_height_m(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    return (int64_t)((g_faces[face]->size->metrics.height * 1000) / 64);
}

int64_t ori_ft_has_kerning(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    return FT_HAS_KERNING(g_faces[face]) ? 1 : 0;
}

/* Kerning between two glyph indices; x component milli-pixel. */
int64_t ori_ft_kerning_x_m(int64_t face, int64_t left_gi, int64_t right_gi) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    if (!FT_HAS_KERNING(g_faces[face])) return 0;
    FT_Vector delta;
    if (FT_Get_Kerning(g_faces[face], (FT_UInt)left_gi, (FT_UInt)right_gi,
                       FT_KERNING_DEFAULT, &delta) != 0) {
        return 0;
    }
    return (int64_t)((delta.x * 1000) / 64);
}

/* Sample last glyph grayscale bitmap at (x,y); 0 if OOB. */
int64_t ori_ft_bitmap_sample(int64_t x, int64_t y) {
    if (x < 0 || y < 0 || x >= g_bmp_w || y >= g_bmp_h) {
        return 0;
    }
    int pitch = g_bmp_pitch > 0 ? g_bmp_pitch : g_bmp_w;
    return (int64_t)g_bitmap[(int)y * pitch + (int)x];
}

/* Units-per-EM of the face (0 if invalid). */
int64_t ori_ft_units_per_em(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    return (int64_t)g_faces[face]->units_per_EM;
}

/* Number of faces in the font file. */
int64_t ori_ft_num_faces(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    return (int64_t)g_faces[face]->num_faces;
}

/* Glyph count in the face. */
int64_t ori_ft_num_glyphs(int64_t face) {
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) return 0;
    return (int64_t)g_faces[face]->num_glyphs;
}

/* ---- ASCII / codepoint range atlas (gray8) ---- */
#define ATLAS_CAP (2048 * 2048)
#define ATLAS_MAX_GLYPHS 256

static unsigned char g_atlas[ATLAS_CAP];
static int g_atlas_w = 0;
static int g_atlas_h = 0;
static int g_atlas_count = 0;
static int g_atlas_cp[ATLAS_MAX_GLYPHS];
static int g_atlas_gi[ATLAS_MAX_GLYPHS];
static int g_atlas_x[ATLAS_MAX_GLYPHS];
static int g_atlas_y[ATLAS_MAX_GLYPHS];
static int g_atlas_gw[ATLAS_MAX_GLYPHS];
static int g_atlas_gh[ATLAS_MAX_GLYPHS];
static int g_atlas_bx[ATLAS_MAX_GLYPHS];
static int g_atlas_by[ATLAS_MAX_GLYPHS];
static int g_atlas_adv[ATLAS_MAX_GLYPHS];

/*
 * Pack glyphs [first_cp, last_cp] inclusive into a gray atlas (shelf packer).
 * Returns glyph count or -1.
 */
int64_t ori_ft_atlas_build(int64_t face, int64_t first_cp, int64_t last_cp, int64_t padding) {
    g_atlas_w = 0;
    g_atlas_h = 0;
    g_atlas_count = 0;
    if (face < 0 || face >= MAX_FACES || !g_faces[face]) {
        set_err("invalid face");
        return -1;
    }
    if (last_cp < first_cp || padding < 0) {
        set_err("bad range");
        return -1;
    }
    FT_Face f = g_faces[face];
    int pad = (int)padding;
    int shelf_x = pad;
    int shelf_y = pad;
    int shelf_h = 0;
    int max_w = 512;
    int max_h = pad;

    for (int64_t cp = first_cp; cp <= last_cp; ++cp) {
        if (g_atlas_count >= ATLAS_MAX_GLYPHS) break;
        FT_UInt gi = FT_Get_Char_Index(f, (FT_ULong)cp);
        if (gi == 0) continue;
        if (FT_Load_Glyph(f, gi, FT_LOAD_DEFAULT) != 0) continue;
        if (FT_Render_Glyph(f->glyph, FT_RENDER_MODE_NORMAL) != 0) continue;
        FT_Bitmap *bm = &f->glyph->bitmap;
        int gw = (int)bm->width;
        int gh = (int)bm->rows;
        int pitch = (int)bm->pitch;
        if (pitch < 0) pitch = -pitch;
        if (shelf_x + gw + pad > max_w) {
            shelf_x = pad;
            shelf_y += shelf_h + pad;
            shelf_h = 0;
        }
        if (gh + pad > shelf_h) shelf_h = gh + pad;
        int need_h = shelf_y + gh + pad;
        if (need_h > max_h) max_h = need_h;
        if (max_h * max_w > ATLAS_CAP) {
            set_err("atlas too large");
            return -1;
        }
        int slot = g_atlas_count++;
        g_atlas_cp[slot] = (int)cp;
        g_atlas_gi[slot] = (int)gi;
        g_atlas_x[slot] = shelf_x;
        g_atlas_y[slot] = shelf_y;
        g_atlas_gw[slot] = gw;
        g_atlas_gh[slot] = gh;
        g_atlas_bx[slot] = (int)f->glyph->bitmap_left;
        g_atlas_by[slot] = (int)f->glyph->bitmap_top;
        g_atlas_adv[slot] = (int)((f->glyph->advance.x * 1000) / 64);
        /* blit later after dimensions known — store temp in g_bitmap style */
        /* We blit immediately into provisional max_w rows; grow width if needed */
        if (shelf_x + gw + pad > g_atlas_w) g_atlas_w = shelf_x + gw + pad;
        shelf_x += gw + pad;
    }
    g_atlas_h = max_h;
    if (g_atlas_w < 1) g_atlas_w = 1;
    if (g_atlas_h < 1) g_atlas_h = 1;
    if (g_atlas_w * g_atlas_h > ATLAS_CAP) {
        set_err("atlas capacity");
        return -1;
    }
    memset(g_atlas, 0, (size_t)(g_atlas_w * g_atlas_h));

    /* Second pass: re-render and blit */
    for (int i = 0; i < g_atlas_count; ++i) {
        FT_UInt gi = (FT_UInt)g_atlas_gi[i];
        if (FT_Load_Glyph(f, gi, FT_LOAD_DEFAULT) != 0) continue;
        if (FT_Render_Glyph(f->glyph, FT_RENDER_MODE_NORMAL) != 0) continue;
        FT_Bitmap *bm = &f->glyph->bitmap;
        int gw = (int)bm->width;
        int gh = (int)bm->rows;
        int pitch = (int)bm->pitch;
        int abs_pitch = pitch < 0 ? -pitch : pitch;
        for (int y = 0; y < gh; ++y) {
            const unsigned char *row = bm->buffer + y * abs_pitch;
            unsigned char *dst = g_atlas + (g_atlas_y[i] + y) * g_atlas_w + g_atlas_x[i];
            memcpy(dst, row, (size_t)gw);
        }
    }
    set_err(NULL);
    return (int64_t)g_atlas_count;
}

int64_t ori_ft_atlas_width(void) { return g_atlas_w; }
int64_t ori_ft_atlas_height(void) { return g_atlas_h; }
int64_t ori_ft_atlas_count(void) { return g_atlas_count; }
int64_t ori_ft_atlas_pixels_ptr(void) { return (int64_t)(intptr_t)g_atlas; }

int64_t ori_ft_atlas_codepoint(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_cp[i];
}
int64_t ori_ft_atlas_glyph_index(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_gi[i];
}
int64_t ori_ft_atlas_x(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_x[i];
}
int64_t ori_ft_atlas_y(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_y[i];
}
int64_t ori_ft_atlas_gw(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_gw[i];
}
int64_t ori_ft_atlas_gh(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_gh[i];
}
int64_t ori_ft_atlas_bearing_x(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_bx[i];
}
int64_t ori_ft_atlas_bearing_y(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_by[i];
}
int64_t ori_ft_atlas_advance_m(int64_t i) {
    if (i < 0 || i >= g_atlas_count) return 0;
    return g_atlas_adv[i];
}

int64_t ori_ft_atlas_find(int64_t codepoint) {
    for (int i = 0; i < g_atlas_count; ++i) {
        if (g_atlas_cp[i] == (int)codepoint) return i;
    }
    return -1;
}

int64_t ori_ft_atlas_ink_sum(void) {
    int64_t sum = 0;
    int n = g_atlas_w * g_atlas_h;
    for (int i = 0; i < n; ++i) sum += g_atlas[i];
    return sum;
}

int64_t ori_ft_atlas_write_pgm(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path || g_atlas_w <= 0 || g_atlas_h <= 0) {
        set_err("no atlas");
        return 1;
    }
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        set_err("fopen failed");
        return 1;
    }
    fprintf(fp, "P5\n%d %d\n255\n", g_atlas_w, g_atlas_h);
    fwrite(g_atlas, 1, (size_t)(g_atlas_w * g_atlas_h), fp);
    fclose(fp);
    set_err(NULL);
    return 0;
}
