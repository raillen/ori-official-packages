/* stb bindings for Ori — int64 ABI for pointers; milli-float for noise. */
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_PERLIN_IMPLEMENTATION
#include "../vendor/stb_image.h"
#include "../vendor/stb_image_write.h"
#include "../vendor/stb_rect_pack.h"
#include "../vendor/stb_perlin.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *g_img = NULL;
static int g_w = 0, g_h = 0, g_comp = 0;

int64_t ori_stb_load(int64_t path_ptr, int64_t desired_channels) {
    if (g_img) {
        stbi_image_free(g_img);
        g_img = NULL;
    }
    g_w = g_h = g_comp = 0;
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    int w = 0, h = 0, n = 0;
    g_img = stbi_load(path, &w, &h, &n, (int)desired_channels);
    if (!g_img) return 1;
    g_w = w;
    g_h = h;
    g_comp = desired_channels > 0 ? (int)desired_channels : n;
    return 0;
}

int64_t ori_stb_width(void) { return g_w; }
int64_t ori_stb_height(void) { return g_h; }
int64_t ori_stb_comp(void) { return g_comp; }
int64_t ori_stb_pixels_ptr(void) { return (int64_t)(intptr_t)g_img; }

int64_t ori_stb_pixel_sum(void) {
    if (!g_img) return 0;
    int64_t sum = 0;
    int n = g_w * g_h * g_comp;
    for (int i = 0; i < n; ++i) sum += g_img[i];
    return sum;
}

void ori_stb_free(void) {
    if (g_img) {
        stbi_image_free(g_img);
        g_img = NULL;
    }
    g_w = g_h = g_comp = 0;
}

/* Write gray8 buffer as PNG (1 channel). */
int64_t ori_stb_write_png(int64_t path_ptr, int64_t w, int64_t h, int64_t data_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    const unsigned char *data = (const unsigned char *)(intptr_t)data_ptr;
    if (!path || !data || w <= 0 || h <= 0) return 1;
    return stbi_write_png(path, (int)w, (int)h, 1, data, (int)w) ? 0 : 1;
}

/* Allocate solid gray buffer, write PNG, free — convenient for tests/smoke. */
int64_t ori_stb_write_solid_gray_png(int64_t path_ptr, int64_t w, int64_t h, int64_t gray) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path || w <= 0 || h <= 0) return 1;
    size_t n = (size_t)w * (size_t)h;
    unsigned char *buf = (unsigned char *)malloc(n);
    if (!buf) return 1;
    memset(buf, (int)(gray & 0xFF), n);
    int ok = stbi_write_png(path, (int)w, (int)h, 1, buf, (int)w);
    free(buf);
    return ok ? 0 : 1;
}

/* Perlin noise * 1000 as int (classic 3D sample). */
int64_t ori_stb_perlin_m(int64_t x_m, int64_t y_m, int64_t z_m) {
    float x = (float)x_m / 1000.0f;
    float y = (float)y_m / 1000.0f;
    float z = (float)z_m / 1000.0f;
    float v = stb_perlin_noise3(x, y, z, 0, 0, 0);
    return (int64_t)(v * 1000.0f);
}

/* 2D sample = 3D with z = 0 (milli in/out). */
int64_t ori_stb_perlin2_m(int64_t x_m, int64_t y_m) {
    float x = (float)x_m / 1000.0f;
    float y = (float)y_m / 1000.0f;
    float v = stb_perlin_noise3(x, y, 0.0f, 0, 0, 0);
    return (int64_t)(v * 1000.0f);
}

/* Rect pack: fill input w/h pairs, return packed x/y. Max 64 rects. */
#define RP_MAX 64
static stbrp_rect g_rects[RP_MAX];
static int g_rect_n = 0;

int64_t ori_stb_pack_begin(int64_t count) {
    g_rect_n = 0;
    if (count < 0 || count > RP_MAX) return 1;
    return 0;
}

int64_t ori_stb_pack_add(int64_t w, int64_t h) {
    if (g_rect_n >= RP_MAX) return 1;
    g_rects[g_rect_n].id = g_rect_n;
    g_rects[g_rect_n].w = (stbrp_coord)w;
    g_rects[g_rect_n].h = (stbrp_coord)h;
    g_rects[g_rect_n].x = 0;
    g_rects[g_rect_n].y = 0;
    g_rects[g_rect_n].was_packed = 0;
    g_rect_n++;
    return 0;
}

int64_t ori_stb_pack_run(int64_t atlas_w, int64_t atlas_h) {
    stbrp_context ctx;
    stbrp_node nodes[512];
    stbrp_init_target(&ctx, (int)atlas_w, (int)atlas_h, nodes, 512);
    stbrp_pack_rects(&ctx, g_rects, g_rect_n);
    int ok = 0;
    for (int i = 0; i < g_rect_n; ++i) if (g_rects[i].was_packed) ok++;
    return ok;
}

int64_t ori_stb_pack_x(int64_t i) {
    if (i < 0 || i >= g_rect_n) return 0;
    return g_rects[i].x;
}
int64_t ori_stb_pack_y(int64_t i) {
    if (i < 0 || i >= g_rect_n) return 0;
    return g_rects[i].y;
}
int64_t ori_stb_pack_w(int64_t i) {
    if (i < 0 || i >= g_rect_n) return 0;
    return g_rects[i].w;
}
int64_t ori_stb_pack_h(int64_t i) {
    if (i < 0 || i >= g_rect_n) return 0;
    return g_rects[i].h;
}
int64_t ori_stb_pack_was(int64_t i) {
    if (i < 0 || i >= g_rect_n) return 0;
    return g_rects[i].was_packed ? 1 : 0;
}
int64_t ori_stb_pack_count(void) {
    return g_rect_n;
}
