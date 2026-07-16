/* ImPlot for Ori.
 * Headless: series buffer + axis labels + draw-kind bookkeeping (tests/smoke).
 * ORI_IMPLOT_FULL: real ImPlot draw (needs active ImGui context at call time).
 */
#include <stdint.h>
#include <string.h>

#define MAX_SAMPLES 2048
#define MAX_LABEL 128

static double g_xs[MAX_SAMPLES];
static double g_ys[MAX_SAMPLES];
static int g_n = 0;

static char g_x_label[MAX_LABEL];
static char g_y_label[MAX_LABEL];
static int g_x_label_len = 0;
static int g_y_label_len = 0;

/* 0=none, 1=line, 2=scatter, 3=bars */
static int g_last_draw = 0;
static int g_plot_open = 0;
static int g_begin_ok = 0;

static void copy_label(char *dst, int *len_out, const char *src) {
    if (!src) {
        dst[0] = '\0';
        *len_out = 0;
        return;
    }
    size_t n = strlen(src);
    if (n >= MAX_LABEL) n = MAX_LABEL - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
    *len_out = (int)n;
}

extern "C" void ori_implot_clear_series(void) {
    g_n = 0;
    g_last_draw = 0;
    g_plot_open = 0;
    g_begin_ok = 0;
}

extern "C" void ori_implot_reset(void) {
    ori_implot_clear_series();
    g_x_label[0] = '\0';
    g_y_label[0] = '\0';
    g_x_label_len = 0;
    g_y_label_len = 0;
}

extern "C" int64_t ori_implot_push_point_m(int64_t x_m, int64_t y_m) {
    if (g_n >= MAX_SAMPLES) return 1;
    g_xs[g_n] = (double)x_m / 1000.0;
    g_ys[g_n] = (double)y_m / 1000.0;
    g_n++;
    return 0;
}

extern "C" int64_t ori_implot_series_count(void) { return g_n; }

extern "C" int64_t ori_implot_series_sum_m(void) {
    double s = 0;
    for (int i = 0; i < g_n; ++i) s += g_ys[i];
    return (int64_t)(s * 1000.0);
}

extern "C" int64_t ori_implot_series_min_m(void) {
    if (g_n <= 0) return 0;
    double m = g_ys[0];
    for (int i = 1; i < g_n; ++i)
        if (g_ys[i] < m) m = g_ys[i];
    return (int64_t)(m * 1000.0);
}

extern "C" int64_t ori_implot_series_max_m(void) {
    if (g_n <= 0) return 0;
    double m = g_ys[0];
    for (int i = 1; i < g_n; ++i)
        if (g_ys[i] > m) m = g_ys[i];
    return (int64_t)(m * 1000.0);
}

extern "C" int64_t ori_implot_series_capacity(void) { return MAX_SAMPLES; }

extern "C" int64_t ori_implot_last_draw(void) { return g_last_draw; }

extern "C" int64_t ori_implot_plot_was_open(void) { return g_plot_open; }

extern "C" int64_t ori_implot_begin_ok(void) { return g_begin_ok; }

extern "C" void ori_implot_setup_axes(int64_t x_label_ptr, int64_t y_label_ptr) {
    const char *x = x_label_ptr ? (const char *)(intptr_t)x_label_ptr : "";
    const char *y = y_label_ptr ? (const char *)(intptr_t)y_label_ptr : "";
    copy_label(g_x_label, &g_x_label_len, x);
    copy_label(g_y_label, &g_y_label_len, y);
}

extern "C" int64_t ori_implot_x_label_len(void) { return g_x_label_len; }
extern "C" int64_t ori_implot_y_label_len(void) { return g_y_label_len; }

extern "C" int64_t ori_implot_x_label_eq(int64_t ptr) {
    const char *s = ptr ? (const char *)(intptr_t)ptr : "";
    return strcmp(g_x_label, s) == 0 ? 1 : 0;
}

extern "C" int64_t ori_implot_y_label_eq(int64_t ptr) {
    const char *s = ptr ? (const char *)(intptr_t)ptr : "";
    return strcmp(g_y_label, s) == 0 ? 1 : 0;
}

extern "C" int64_t ori_implot_is_full_build(void) {
#if defined(ORI_IMPLOT_FULL)
    return 1;
#else
    return 0;
#endif
}

#if defined(ORI_IMPLOT_FULL)
#include "imgui.h"
#include "../vendor/implot.h"

extern "C" void ori_implot_create_context(void) {
    if (!ImPlot::GetCurrentContext()) ImPlot::CreateContext();
}

extern "C" void ori_implot_destroy_context(void) {
    if (ImPlot::GetCurrentContext()) ImPlot::DestroyContext();
}

extern "C" int64_t ori_implot_begin_plot(int64_t title_ptr, int64_t w, int64_t h) {
    g_plot_open = 1;
    g_begin_ok = 0;
    if (!ImGui::GetCurrentContext()) return 1;
    const char *title = title_ptr ? (const char *)(intptr_t)title_ptr : "plot";
    ImVec2 size((float)(w > 0 ? w : -1), (float)(h > 0 ? h : 0));
    if (!ImPlot::BeginPlot(title, size)) return 1;
    if (g_x_label_len > 0 || g_y_label_len > 0) {
        ImPlot::SetupAxes(
            g_x_label_len > 0 ? g_x_label : nullptr,
            g_y_label_len > 0 ? g_y_label : nullptr);
    }
    g_begin_ok = 1;
    return 0;
}

extern "C" void ori_implot_plot_line(int64_t label_ptr) {
    g_last_draw = 1;
    if (!g_begin_ok || g_n <= 0) return;
    const char *label = label_ptr ? (const char *)(intptr_t)label_ptr : "series";
    ImPlot::PlotLine(label, g_xs, g_ys, g_n);
}

extern "C" void ori_implot_plot_scatter(int64_t label_ptr) {
    g_last_draw = 2;
    if (!g_begin_ok || g_n <= 0) return;
    const char *label = label_ptr ? (const char *)(intptr_t)label_ptr : "scatter";
    ImPlot::PlotScatter(label, g_xs, g_ys, g_n);
}

extern "C" void ori_implot_plot_bars(int64_t label_ptr) {
    g_last_draw = 3;
    if (!g_begin_ok || g_n <= 0) return;
    const char *label = label_ptr ? (const char *)(intptr_t)label_ptr : "bars";
    ImPlot::PlotBars(label, g_ys, g_n, 0.67);
}

extern "C" void ori_implot_end_plot(void) {
    if (g_begin_ok) ImPlot::EndPlot();
    g_begin_ok = 0;
    g_plot_open = 0;
}
#else
extern "C" void ori_implot_create_context(void) {}
extern "C" void ori_implot_destroy_context(void) {}

extern "C" int64_t ori_implot_begin_plot(int64_t, int64_t, int64_t) {
    /* Headless: track open; cannot actually draw without ImGui. */
    g_plot_open = 1;
    g_begin_ok = 0;
    return 1;
}

extern "C" void ori_implot_plot_line(int64_t) {
    g_last_draw = 1;
}

extern "C" void ori_implot_plot_scatter(int64_t) {
    g_last_draw = 2;
}

extern "C" void ori_implot_plot_bars(int64_t) {
    g_last_draw = 3;
}

extern "C" void ori_implot_end_plot(void) {
    g_plot_open = 0;
    g_begin_ok = 0;
}
#endif
