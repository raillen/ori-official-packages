/* ImZoomSlider view range (milli) + optional FULL interactive slider.
 *
 * Milli: range and view bounds as float * 1000.
 * "Zoom value" = view span (view_higher - view_lower) in milli.
 */
#include <stdint.h>
#include <math.h>

static float g_lower = 0.0f;
static float g_higher = 1.0f;
static float g_view_lower = 0.0f;
static float g_view_higher = 1.0f;

static int64_t m_from_f(float v) {
    return (int64_t)llroundf(v * 1000.0f);
}

static float f_from_m(int64_t m) {
    return (float)m / 1000.0f;
}

static void clip_view(void) {
    if (g_higher <= g_lower) {
        g_higher = g_lower + 0.001f;
    }
    float span = g_view_higher - g_view_lower;
    if (span < 0.0f) {
        float tmp = g_view_lower;
        g_view_lower = g_view_higher;
        g_view_higher = tmp;
        span = g_view_higher - g_view_lower;
    }
    if (span < 1e-6f) {
        span = (g_higher - g_lower) * 0.1f;
        if (span < 1e-6f) span = 0.001f;
        const float mid = (g_view_lower + g_view_higher) * 0.5f;
        g_view_lower = mid - span * 0.5f;
        g_view_higher = mid + span * 0.5f;
    }
    if (g_view_lower < g_lower) {
        const float d = g_lower - g_view_lower;
        g_view_lower += d;
        g_view_higher += d;
    }
    if (g_view_higher > g_higher) {
        const float d = g_view_higher - g_higher;
        g_view_lower -= d;
        g_view_higher -= d;
    }
    if (g_view_lower < g_lower) g_view_lower = g_lower;
    if (g_view_higher > g_higher) g_view_higher = g_higher;
}

extern "C" void ori_zoom_reset(void) {
    g_lower = 0.0f;
    g_higher = 1.0f;
    g_view_lower = 0.0f;
    g_view_higher = 1.0f;
}

extern "C" void ori_zoom_set_range_m(int64_t lower_m, int64_t higher_m) {
    g_lower = f_from_m(lower_m);
    g_higher = f_from_m(higher_m);
    if (g_higher <= g_lower) {
        g_higher = g_lower + 0.001f;
    }
    clip_view();
}

extern "C" void ori_zoom_set_view_m(int64_t view_lower_m, int64_t view_higher_m) {
    g_view_lower = f_from_m(view_lower_m);
    g_view_higher = f_from_m(view_higher_m);
    clip_view();
}

extern "C" int64_t ori_zoom_lower_m(void) { return m_from_f(g_lower); }
extern "C" int64_t ori_zoom_higher_m(void) { return m_from_f(g_higher); }
extern "C" int64_t ori_zoom_view_lower_m(void) { return m_from_f(g_view_lower); }
extern "C" int64_t ori_zoom_view_higher_m(void) { return m_from_f(g_view_higher); }

/* Zoom value = current view span in milli (smaller span ⇒ more zoomed in). */
extern "C" int64_t ori_zoom_span_m(void) {
    return m_from_f(g_view_higher - g_view_lower);
}

#if defined(ORI_IMGUIZMO_FULL)
#include "imgui.h"
#include "imgui_internal.h"
#include "../vendor/ImZoomSlider.h"

extern "C" int64_t ori_zoom_slider(void) {
    if (ImGui::GetCurrentContext() == nullptr) {
        return 0;
    }
    float view_lo = g_view_lower;
    float view_hi = g_view_higher;
    bool changed = ImZoomSlider::ImZoomSlider(
        g_lower, g_higher, view_lo, view_hi, 0.01f, ImZoomSlider::ImGuiZoomSliderFlags_None);
    g_view_lower = view_lo;
    g_view_higher = view_hi;
    clip_view();
    return changed ? 1 : 0;
}
#else
extern "C" int64_t ori_zoom_slider(void) {
    /* Headless: no interaction; view remains set via set_view_m. */
    return 0;
}
#endif
