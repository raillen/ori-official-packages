/* ImGradient color stops (milli) + optional FULL ImGradient::Edit.
 *
 * Stop layout matches upstream: rgba in xyzw where w = position t in [0,1].
 * Milli: channel and t as float * 1000 (1000 = 1.0).
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GRAD_MAX_STOPS 32

struct GradStop {
    float r, g, b, a; /* a is alpha; position stored separately as t */
    float t;
};

static GradStop g_stops[GRAD_MAX_STOPS];
static int g_stop_n = 0;

static int64_t m_from_f(float v) {
    return (int64_t)llroundf(v * 1000.0f);
}

static float f_from_m(int64_t m) {
    return (float)m / 1000.0f;
}

static void sort_stops(void) {
    for (int i = 1; i < g_stop_n; ++i) {
        GradStop key = g_stops[i];
        int j = i - 1;
        while (j >= 0 && g_stops[j].t > key.t) {
            g_stops[j + 1] = g_stops[j];
            --j;
        }
        g_stops[j + 1] = key;
    }
}

extern "C" void ori_gradient_clear(void) {
    g_stop_n = 0;
}

extern "C" int64_t ori_gradient_stop_count(void) {
    return (int64_t)g_stop_n;
}

extern "C" int64_t ori_gradient_add_stop_m(
    int64_t r_m, int64_t g_m, int64_t b_m, int64_t a_m, int64_t t_m) {
    if (g_stop_n >= GRAD_MAX_STOPS) {
        return -1;
    }
    GradStop s;
    s.r = f_from_m(r_m);
    s.g = f_from_m(g_m);
    s.b = f_from_m(b_m);
    s.a = f_from_m(a_m);
    s.t = f_from_m(t_m);
    if (s.t < 0.f) s.t = 0.f;
    if (s.t > 1.f) s.t = 1.f;
    g_stops[g_stop_n++] = s;
    sort_stops();
    for (int i = 0; i < g_stop_n; ++i) {
        if (fabsf(g_stops[i].t - s.t) < 1e-6f &&
            fabsf(g_stops[i].r - s.r) < 1e-6f) {
            return (int64_t)i;
        }
    }
    return (int64_t)(g_stop_n - 1);
}

extern "C" int64_t ori_gradient_set_stop_m(
    int64_t index,
    int64_t r_m, int64_t g_m, int64_t b_m, int64_t a_m, int64_t t_m) {
    if (index < 0 || index >= g_stop_n) {
        return 1;
    }
    GradStop& s = g_stops[(int)index];
    s.r = f_from_m(r_m);
    s.g = f_from_m(g_m);
    s.b = f_from_m(b_m);
    s.a = f_from_m(a_m);
    s.t = f_from_m(t_m);
    if (s.t < 0.f) s.t = 0.f;
    if (s.t > 1.f) s.t = 1.f;
    sort_stops();
    return 0;
}

static int valid_stop(int64_t index) {
    return index >= 0 && index < g_stop_n;
}

extern "C" int64_t ori_gradient_stop_r_m(int64_t index) {
    return valid_stop(index) ? m_from_f(g_stops[(int)index].r) : 0;
}
extern "C" int64_t ori_gradient_stop_g_m(int64_t index) {
    return valid_stop(index) ? m_from_f(g_stops[(int)index].g) : 0;
}
extern "C" int64_t ori_gradient_stop_b_m(int64_t index) {
    return valid_stop(index) ? m_from_f(g_stops[(int)index].b) : 0;
}
extern "C" int64_t ori_gradient_stop_a_m(int64_t index) {
    return valid_stop(index) ? m_from_f(g_stops[(int)index].a) : 0;
}
extern "C" int64_t ori_gradient_stop_t_m(int64_t index) {
    return valid_stop(index) ? m_from_f(g_stops[(int)index].t) : 0;
}

/* Sample RGBA at t in [0,1] milli; empty gradient → black transparent. */
static void sample_rgba(float t, float* out_r, float* out_g, float* out_b, float* out_a) {
    if (g_stop_n <= 0) {
        *out_r = *out_g = *out_b = *out_a = 0.f;
        return;
    }
    if (g_stop_n == 1 || t <= g_stops[0].t) {
        *out_r = g_stops[0].r;
        *out_g = g_stops[0].g;
        *out_b = g_stops[0].b;
        *out_a = g_stops[0].a;
        return;
    }
    if (t >= g_stops[g_stop_n - 1].t) {
        const GradStop& s = g_stops[g_stop_n - 1];
        *out_r = s.r;
        *out_g = s.g;
        *out_b = s.b;
        *out_a = s.a;
        return;
    }
    for (int i = 0; i < g_stop_n - 1; ++i) {
        const GradStop& a = g_stops[i];
        const GradStop& b = g_stops[i + 1];
        if (t >= a.t && t <= b.t) {
            const float dt = b.t - a.t;
            const float u = (fabsf(dt) < 1e-8f) ? 0.f : (t - a.t) / dt;
            *out_r = a.r + (b.r - a.r) * u;
            *out_g = a.g + (b.g - a.g) * u;
            *out_b = a.b + (b.b - a.b) * u;
            *out_a = a.a + (b.a - a.a) * u;
            return;
        }
    }
    const GradStop& s = g_stops[g_stop_n - 1];
    *out_r = s.r;
    *out_g = s.g;
    *out_b = s.b;
    *out_a = s.a;
}

extern "C" int64_t ori_gradient_sample_r_m(int64_t t_m) {
    float r, g, b, a;
    sample_rgba(f_from_m(t_m), &r, &g, &b, &a);
    return m_from_f(r);
}
extern "C" int64_t ori_gradient_sample_g_m(int64_t t_m) {
    float r, g, b, a;
    sample_rgba(f_from_m(t_m), &r, &g, &b, &a);
    return m_from_f(g);
}
extern "C" int64_t ori_gradient_sample_b_m(int64_t t_m) {
    float r, g, b, a;
    sample_rgba(f_from_m(t_m), &r, &g, &b, &a);
    return m_from_f(b);
}
extern "C" int64_t ori_gradient_sample_a_m(int64_t t_m) {
    float r, g, b, a;
    sample_rgba(f_from_m(t_m), &r, &g, &b, &a);
    return m_from_f(a);
}

#if defined(ORI_IMGUIZMO_FULL)
#include "imgui.h"
#include "imgui_internal.h"
#include "../vendor/ImGradient.h"

/* Upstream ImGradient stores ImVec4 as rgb + w=position. Alpha fixed at 1 for UI. */
struct OriGradientDelegate : public ImGradient::Delegate {
    ImVec4 pts[GRAD_MAX_STOPS];

    void sync_from_global() {
        for (int i = 0; i < g_stop_n; ++i) {
            pts[i] = ImVec4(g_stops[i].r, g_stops[i].g, g_stops[i].b, g_stops[i].t);
        }
    }

    void sync_to_global() {
        for (int i = 0; i < g_stop_n; ++i) {
            g_stops[i].r = pts[i].x;
            g_stops[i].g = pts[i].y;
            g_stops[i].b = pts[i].z;
            g_stops[i].t = pts[i].w;
            if (g_stops[i].a <= 0.f) {
                g_stops[i].a = 1.f;
            }
        }
        sort_stops();
    }

    size_t GetPointCount() override { return (size_t)g_stop_n; }
    ImVec4* GetPoints() override { return pts; }

    int EditPoint(int pointIndex, ImVec4 value) override {
        if (pointIndex < 0 || pointIndex >= g_stop_n) {
            return 0;
        }
        pts[pointIndex] = value;
        g_stops[pointIndex].r = value.x;
        g_stops[pointIndex].g = value.y;
        g_stops[pointIndex].b = value.z;
        g_stops[pointIndex].t = value.w;
        sort_stops();
        sync_from_global();
        return pointIndex;
    }

    ImVec4 GetPoint(float t) override {
        float r, g, b, a;
        sample_rgba(t, &r, &g, &b, &a);
        return ImVec4(r, g, b, t);
    }

    void AddPoint(ImVec4 value) override {
        if (g_stop_n >= GRAD_MAX_STOPS) {
            return;
        }
        GradStop s;
        s.r = value.x;
        s.g = value.y;
        s.b = value.z;
        s.a = 1.f;
        s.t = value.w;
        g_stops[g_stop_n++] = s;
        sort_stops();
        sync_from_global();
    }
};

extern "C" int64_t ori_gradient_edit(void) {
    if (ImGui::GetCurrentContext() == nullptr) {
        return 0;
    }
    static OriGradientDelegate del;
    del.sync_from_global();
    int selection = -1;
    bool changed = ImGradient::Edit(del, ImVec2(400.f, 40.f), selection);
    del.sync_to_global();
    return changed ? 1 : 0;
}
#else
extern "C" int64_t ori_gradient_edit(void) {
    return 0;
}
#endif
