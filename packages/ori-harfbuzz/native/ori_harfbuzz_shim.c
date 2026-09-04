/* HarfBuzz shaping shim for Ori — borrowed FT_Face* (int64). */
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GLYPHS 512

/* Direction codes (match Ori constants). */
#define ORI_HB_DIR_LTR 0
#define ORI_HB_DIR_RTL 1
#define ORI_HB_DIR_TTB 2
#define ORI_HB_DIR_BTT 3
#define ORI_HB_DIR_AUTO 4

static hb_glyph_info_t g_infos[MAX_GLYPHS];
static hb_glyph_position_t g_pos[MAX_GLYPHS];
static int g_count = 0;
static char g_err[256];

static void set_err(const char *msg) {
    if (!msg) {
        g_err[0] = '\0';
        return;
    }
    strncpy(g_err, msg, sizeof(g_err) - 1);
    g_err[sizeof(g_err) - 1] = '\0';
}

static hb_direction_t dir_from_code(int64_t d) {
    if (d == ORI_HB_DIR_RTL) return HB_DIRECTION_RTL;
    if (d == ORI_HB_DIR_TTB) return HB_DIRECTION_TTB;
    if (d == ORI_HB_DIR_BTT) return HB_DIRECTION_BTT;
    if (d == ORI_HB_DIR_LTR) return HB_DIRECTION_LTR;
    return HB_DIRECTION_INVALID; /* auto / guess */
}

/*
 * face_ptr: FT_Face as int64
 * text_ptr: UTF-8 C string
 * direction: ORI_HB_DIR_*
 * Returns glyph count or -1.
 */
int64_t ori_hb_shape_utf8_dir(int64_t face_ptr, int64_t text_ptr, int64_t direction) {
    g_count = 0;
    FT_Face ft_face = (FT_Face)(intptr_t)face_ptr;
    const char *text = (const char *)(intptr_t)text_ptr;
    if (!ft_face || !text) {
        set_err("null face or text");
        return -1;
    }

    hb_font_t *font = hb_ft_font_create_referenced(ft_face);
    if (!font) {
        set_err("hb_ft_font_create failed");
        return -1;
    }
    hb_ft_font_set_load_flags(font, FT_LOAD_DEFAULT);

    hb_buffer_t *buf = hb_buffer_create();
    if (!buf) {
        hb_font_destroy(font);
        set_err("hb_buffer_create failed");
        return -1;
    }

    hb_buffer_add_utf8(buf, text, -1, 0, -1);
    hb_direction_t dir = dir_from_code(direction);
    if (dir == HB_DIRECTION_INVALID) {
        hb_buffer_guess_segment_properties(buf);
    } else {
        hb_buffer_set_direction(buf, dir);
        hb_buffer_guess_segment_properties(buf);
        hb_buffer_set_direction(buf, dir);
    }
    hb_shape(font, buf, NULL, 0);

    unsigned int len = 0;
    hb_glyph_info_t *infos = hb_buffer_get_glyph_infos(buf, &len);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(buf, &len);
    if (len > MAX_GLYPHS) {
        len = MAX_GLYPHS;
    }
    if (infos && pos && len > 0) {
        memcpy(g_infos, infos, len * sizeof(hb_glyph_info_t));
        memcpy(g_pos, pos, len * sizeof(hb_glyph_position_t));
    }
    g_count = (int)len;

    hb_buffer_destroy(buf);
    hb_font_destroy(font);
    set_err(NULL);
    return (int64_t)g_count;
}

int64_t ori_hb_shape_utf8(int64_t face_ptr, int64_t text_ptr) {
    return ori_hb_shape_utf8_dir(face_ptr, text_ptr, ORI_HB_DIR_AUTO);
}

int64_t ori_hb_glyph_count(void) { return (int64_t)g_count; }

int64_t ori_hb_glyph_id(int64_t index) {
    if (index < 0 || index >= g_count) return 0;
    return (int64_t)g_infos[index].codepoint;
}

int64_t ori_hb_cluster(int64_t index) {
    if (index < 0 || index >= g_count) return 0;
    return (int64_t)g_infos[index].cluster;
}

int64_t ori_hb_x_advance_m(int64_t index) {
    if (index < 0 || index >= g_count) return 0;
    return (int64_t)((g_pos[index].x_advance * 1000) / 64);
}

int64_t ori_hb_y_advance_m(int64_t index) {
    if (index < 0 || index >= g_count) return 0;
    return (int64_t)((g_pos[index].y_advance * 1000) / 64);
}

int64_t ori_hb_x_offset_m(int64_t index) {
    if (index < 0 || index >= g_count) return 0;
    return (int64_t)((g_pos[index].x_offset * 1000) / 64);
}

int64_t ori_hb_y_offset_m(int64_t index) {
    if (index < 0 || index >= g_count) return 0;
    return (int64_t)((g_pos[index].y_offset * 1000) / 64);
}
