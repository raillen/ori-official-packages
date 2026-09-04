/* ImCurveEdit data model (milli) + optional FULL ImCurveEdit::Edit.
 *
 * Headless: point list + linear sample (tests/smoke).
 * FULL (ORI_IMGUIZMO_FULL): real ImCurveEdit UI when ImGui context is active.
 *
 * Milli: time and value as float * 1000 (int64_t ABI).
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CURVE_MAX_POINTS 64

static float g_curve_t[CURVE_MAX_POINTS];
static float g_curve_v[CURVE_MAX_POINTS];
static int g_curve_n = 0;

static int64_t m_from_f(float v) {
    return (int64_t)llroundf(v * 1000.0f);
}

static float f_from_m(int64_t m) {
    return (float)m / 1000.0f;
}

/* Keep points sorted by t ascending after mutations. */
static void sort_curve_points(void) {
    for (int i = 1; i < g_curve_n; ++i) {
        float tt = g_curve_t[i];
        float vv = g_curve_v[i];
        int j = i - 1;
        while (j >= 0 && g_curve_t[j] > tt) {
            g_curve_t[j + 1] = g_curve_t[j];
            g_curve_v[j + 1] = g_curve_v[j];
            --j;
        }
        g_curve_t[j + 1] = tt;
        g_curve_v[j + 1] = vv;
    }
}

extern "C" void ori_curve_clear(void) {
    g_curve_n = 0;
}

extern "C" int64_t ori_curve_point_count(void) {
    return (int64_t)g_curve_n;
}

extern "C" int64_t ori_curve_add_point_m(int64_t t_m, int64_t v_m) {
    if (g_curve_n >= CURVE_MAX_POINTS) {
        return -1;
    }
    int idx = g_curve_n;
    g_curve_t[idx] = f_from_m(t_m);
    g_curve_v[idx] = f_from_m(v_m);
    g_curve_n++;
    sort_curve_points();
    /* Return index after sort. */
    float want_t = f_from_m(t_m);
    float want_v = f_from_m(v_m);
    for (int i = 0; i < g_curve_n; ++i) {
        if (fabsf(g_curve_t[i] - want_t) < 1e-6f && fabsf(g_curve_v[i] - want_v) < 1e-6f) {
            return (int64_t)i;
        }
    }
    return (int64_t)(g_curve_n - 1);
}

extern "C" int64_t ori_curve_set_point_m(int64_t index, int64_t t_m, int64_t v_m) {
    if (index < 0 || index >= g_curve_n) {
        return 1;
    }
    g_curve_t[(int)index] = f_from_m(t_m);
    g_curve_v[(int)index] = f_from_m(v_m);
    sort_curve_points();
    return 0;
}

extern "C" int64_t ori_curve_point_t_m(int64_t index) {
    if (index < 0 || index >= g_curve_n) {
        return 0;
    }
    return m_from_f(g_curve_t[(int)index]);
}

extern "C" int64_t ori_curve_point_v_m(int64_t index) {
    if (index < 0 || index >= g_curve_n) {
        return 0;
    }
    return m_from_f(g_curve_v[(int)index]);
}

/* Piecewise linear sample at t (milli). Empty → 0; one point → that value. */
extern "C" int64_t ori_curve_sample_m(int64_t t_m) {
    if (g_curve_n <= 0) {
        return 0;
    }
    if (g_curve_n == 1) {
        return m_from_f(g_curve_v[0]);
    }
    const float t = f_from_m(t_m);
    if (t <= g_curve_t[0]) {
        return m_from_f(g_curve_v[0]);
    }
    if (t >= g_curve_t[g_curve_n - 1]) {
        return m_from_f(g_curve_v[g_curve_n - 1]);
    }
    for (int i = 0; i < g_curve_n - 1; ++i) {
        const float t0 = g_curve_t[i];
        const float t1 = g_curve_t[i + 1];
        if (t >= t0 && t <= t1) {
            const float dt = t1 - t0;
            if (fabsf(dt) < 1e-8f) {
                return m_from_f(g_curve_v[i]);
            }
            const float u = (t - t0) / dt;
            const float v = g_curve_v[i] + (g_curve_v[i + 1] - g_curve_v[i]) * u;
            return m_from_f(v);
        }
    }
    return m_from_f(g_curve_v[g_curve_n - 1]);
}

#if defined(ORI_IMGUIZMO_FULL)
#include "imgui.h"
#include "imgui_internal.h"
#include "../vendor/ImCurveEdit.h"

struct OriCurveDelegate : public ImCurveEdit::Delegate {
    ImVec2 pts[CURVE_MAX_POINTS];
    ImVec2 range_min;
    ImVec2 range_max;

    OriCurveDelegate() : range_min(0.f, 0.f), range_max(1.f, 1.f) {}

    void sync_from_global() {
        for (int i = 0; i < g_curve_n; ++i) {
            pts[i] = ImVec2(g_curve_t[i], g_curve_v[i]);
        }
        if (g_curve_n >= 1) {
            float min_t = g_curve_t[0], max_t = g_curve_t[0];
            float min_v = g_curve_v[0], max_v = g_curve_v[0];
            for (int i = 1; i < g_curve_n; ++i) {
                if (g_curve_t[i] < min_t) min_t = g_curve_t[i];
                if (g_curve_t[i] > max_t) max_t = g_curve_t[i];
                if (g_curve_v[i] < min_v) min_v = g_curve_v[i];
                if (g_curve_v[i] > max_v) max_v = g_curve_v[i];
            }
            if (max_t - min_t < 1e-4f) {
                max_t = min_t + 1.f;
            }
            if (max_v - min_v < 1e-4f) {
                max_v = min_v + 1.f;
            }
            range_min = ImVec2(min_t, min_v);
            range_max = ImVec2(max_t, max_v);
        } else {
            range_min = ImVec2(0.f, 0.f);
            range_max = ImVec2(1.f, 1.f);
        }
    }

    void sync_to_global() {
        for (int i = 0; i < g_curve_n; ++i) {
            g_curve_t[i] = pts[i].x;
            g_curve_v[i] = pts[i].y;
        }
        sort_curve_points();
    }

    size_t GetCurveCount() override { return 1; }
    size_t GetPointCount(size_t /*curveIndex*/) override {
        return (size_t)g_curve_n;
    }
    uint32_t GetCurveColor(size_t /*curveIndex*/) override { return 0xFF0080FF; }
    ImVec2* GetPoints(size_t /*curveIndex*/) override { return pts; }
    ImVec2& GetMin() override { return range_min; }
    ImVec2& GetMax() override { return range_max; }
    int EditPoint(size_t /*curveIndex*/, int pointIndex, ImVec2 value) override {
        if (pointIndex < 0 || pointIndex >= g_curve_n) {
            return 0;
        }
        pts[pointIndex] = value;
        g_curve_t[pointIndex] = value.x;
        g_curve_v[pointIndex] = value.y;
        sort_curve_points();
        sync_from_global();
        return pointIndex;
    }
    void AddPoint(size_t /*curveIndex*/, ImVec2 value) override {
        if (g_curve_n >= CURVE_MAX_POINTS) {
            return;
        }
        g_curve_t[g_curve_n] = value.x;
        g_curve_v[g_curve_n] = value.y;
        g_curve_n++;
        sort_curve_points();
        sync_from_global();
    }
};

extern "C" int64_t ori_curve_edit(void) {
    if (ImGui::GetCurrentContext() == nullptr) {
        return 0;
    }
    static OriCurveDelegate del;
    del.sync_from_global();
    int ret = ImCurveEdit::Edit(del, ImVec2(400.f, 200.f), 0xC0FFE1u);
    del.sync_to_global();
    return ret != 0 ? 1 : 0;
}
#else
extern "C" int64_t ori_curve_edit(void) {
    /* Headless: no UI; points remain editable via set/add APIs. */
    return 0;
}
#endif
