/* ImGuizmo matrix helpers (TRS milli) + optional FULL Manipulate path.
 *
 * Headless path: bookkeeping TRS components → column-major matrix (no ImGui).
 * FULL path (ORI_IMGUIZMO_FULL): ImGuizmo::Manipulate for TRANSLATE/ROTATE/SCALE.
 *
 * Milli convention (int64_t ABI):
 *   translation: world units * 1000
 *   rotation:    degrees * 1000
 *   scale:       factor * 1000  (1000 = 1.0)
 */
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float g_t[3] = {0.0f, 0.0f, 0.0f};
static float g_r[3] = {0.0f, 0.0f, 0.0f}; /* degrees */
static float g_s[3] = {1.0f, 1.0f, 1.0f};
static float g_matrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

/* last manipulate/op bookkeeping: 0=none, 1=translate, 2=rotate, 3=scale */
static int64_t g_last_op = 0;

static int64_t m_from_f(float v) {
    return (int64_t)llroundf(v * 1000.0f);
}

static float f_from_m(int64_t m) {
    return (float)m / 1000.0f;
}

/* Recompose column-major M = T * R(z*y*x) * S (matches common ImGuizmo-style TRS). */
static void recompose_matrix(void) {
    const float deg2rad = (float)(M_PI / 180.0);
    const float cx = cosf(g_r[0] * deg2rad);
    const float sx = sinf(g_r[0] * deg2rad);
    const float cy = cosf(g_r[1] * deg2rad);
    const float sy = sinf(g_r[1] * deg2rad);
    const float cz = cosf(g_r[2] * deg2rad);
    const float sz = sinf(g_r[2] * deg2rad);

    /* R = Rz * Ry * Rx */
    const float r00 = cy * cz;
    const float r01 = cz * sx * sy - cx * sz;
    const float r02 = cx * cz * sy + sx * sz;
    const float r10 = cy * sz;
    const float r11 = cx * cz + sx * sy * sz;
    const float r12 = -cz * sx + cx * sy * sz;
    const float r20 = -sy;
    const float r21 = cy * sx;
    const float r22 = cx * cy;

    /* columns = R * scale */
    g_matrix[0] = r00 * g_s[0];
    g_matrix[1] = r10 * g_s[0];
    g_matrix[2] = r20 * g_s[0];
    g_matrix[3] = 0.0f;

    g_matrix[4] = r01 * g_s[1];
    g_matrix[5] = r11 * g_s[1];
    g_matrix[6] = r21 * g_s[1];
    g_matrix[7] = 0.0f;

    g_matrix[8] = r02 * g_s[2];
    g_matrix[9] = r12 * g_s[2];
    g_matrix[10] = r22 * g_s[2];
    g_matrix[11] = 0.0f;

    g_matrix[12] = g_t[0];
    g_matrix[13] = g_t[1];
    g_matrix[14] = g_t[2];
    g_matrix[15] = 1.0f;
}

/* Lightweight decompose from our own recompose (column norms + euler extract). */
static void sync_components_from_matrix(void) {
    g_t[0] = g_matrix[12];
    g_t[1] = g_matrix[13];
    g_t[2] = g_matrix[14];

    float sx = sqrtf(g_matrix[0] * g_matrix[0] + g_matrix[1] * g_matrix[1] + g_matrix[2] * g_matrix[2]);
    float sy = sqrtf(g_matrix[4] * g_matrix[4] + g_matrix[5] * g_matrix[5] + g_matrix[6] * g_matrix[6]);
    float sz = sqrtf(g_matrix[8] * g_matrix[8] + g_matrix[9] * g_matrix[9] + g_matrix[10] * g_matrix[10]);
    if (sx < 1e-8f) sx = 1e-8f;
    if (sy < 1e-8f) sy = 1e-8f;
    if (sz < 1e-8f) sz = 1e-8f;
    g_s[0] = sx;
    g_s[1] = sy;
    g_s[2] = sz;

    const float r00 = g_matrix[0] / sx;
    const float r10 = g_matrix[1] / sx;
    const float r20 = g_matrix[2] / sx;
    const float r21 = g_matrix[6] / sy;
    const float r22 = g_matrix[10] / sz;

    g_r[1] = asinf(-r20) * (float)(180.0 / M_PI);
    if (fabsf(r20) < 0.99999f) {
        g_r[0] = atan2f(r21, r22) * (float)(180.0 / M_PI);
        g_r[2] = atan2f(r10, r00) * (float)(180.0 / M_PI);
    } else {
        g_r[0] = 0.0f;
        g_r[2] = atan2f(-g_matrix[4] / sy, g_matrix[5] / sy) * (float)(180.0 / M_PI);
    }
}

extern "C" void ori_gizmo_identity(void) {
    g_t[0] = g_t[1] = g_t[2] = 0.0f;
    g_r[0] = g_r[1] = g_r[2] = 0.0f;
    g_s[0] = g_s[1] = g_s[2] = 1.0f;
    g_last_op = 0;
    recompose_matrix();
}

extern "C" void ori_gizmo_set_translation_m(int64_t x_m, int64_t y_m, int64_t z_m) {
    g_t[0] = f_from_m(x_m);
    g_t[1] = f_from_m(y_m);
    g_t[2] = f_from_m(z_m);
    recompose_matrix();
    g_last_op = 1;
}

extern "C" int64_t ori_gizmo_tx_m(void) { return m_from_f(g_t[0]); }
extern "C" int64_t ori_gizmo_ty_m(void) { return m_from_f(g_t[1]); }
extern "C" int64_t ori_gizmo_tz_m(void) { return m_from_f(g_t[2]); }

extern "C" void ori_gizmo_translate_m(int64_t dx_m, int64_t dy_m, int64_t dz_m) {
    g_t[0] += f_from_m(dx_m);
    g_t[1] += f_from_m(dy_m);
    g_t[2] += f_from_m(dz_m);
    recompose_matrix();
    g_last_op = 1;
}

extern "C" void ori_gizmo_set_rotation_m(int64_t x_m, int64_t y_m, int64_t z_m) {
    g_r[0] = f_from_m(x_m);
    g_r[1] = f_from_m(y_m);
    g_r[2] = f_from_m(z_m);
    recompose_matrix();
    g_last_op = 2;
}

extern "C" int64_t ori_gizmo_rx_m(void) { return m_from_f(g_r[0]); }
extern "C" int64_t ori_gizmo_ry_m(void) { return m_from_f(g_r[1]); }
extern "C" int64_t ori_gizmo_rz_m(void) { return m_from_f(g_r[2]); }

extern "C" void ori_gizmo_rotate_m(int64_t dx_m, int64_t dy_m, int64_t dz_m) {
    g_r[0] += f_from_m(dx_m);
    g_r[1] += f_from_m(dy_m);
    g_r[2] += f_from_m(dz_m);
    recompose_matrix();
    g_last_op = 2;
}

extern "C" void ori_gizmo_set_scale_m(int64_t x_m, int64_t y_m, int64_t z_m) {
    g_s[0] = f_from_m(x_m);
    g_s[1] = f_from_m(y_m);
    g_s[2] = f_from_m(z_m);
    if (g_s[0] == 0.0f) g_s[0] = 1e-6f;
    if (g_s[1] == 0.0f) g_s[1] = 1e-6f;
    if (g_s[2] == 0.0f) g_s[2] = 1e-6f;
    recompose_matrix();
    g_last_op = 3;
}

extern "C" int64_t ori_gizmo_sx_m(void) { return m_from_f(g_s[0]); }
extern "C" int64_t ori_gizmo_sy_m(void) { return m_from_f(g_s[1]); }
extern "C" int64_t ori_gizmo_sz_m(void) { return m_from_f(g_s[2]); }

extern "C" void ori_gizmo_scale_m(int64_t dx_m, int64_t dy_m, int64_t dz_m) {
    g_s[0] += f_from_m(dx_m);
    g_s[1] += f_from_m(dy_m);
    g_s[2] += f_from_m(dz_m);
    if (g_s[0] == 0.0f) g_s[0] = 1e-6f;
    if (g_s[1] == 0.0f) g_s[1] = 1e-6f;
    if (g_s[2] == 0.0f) g_s[2] = 1e-6f;
    recompose_matrix();
    g_last_op = 3;
}

/* Column-major matrix element as milli-float (index 0..15). */
extern "C" int64_t ori_gizmo_matrix_m(int64_t index) {
    if (index < 0 || index > 15) {
        return 0;
    }
    return m_from_f(g_matrix[(int)index]);
}

extern "C" void ori_gizmo_set_matrix_m(int64_t index, int64_t value_m) {
    if (index < 0 || index > 15) {
        return;
    }
    g_matrix[(int)index] = f_from_m(value_m);
    sync_components_from_matrix();
}

extern "C" int64_t ori_gizmo_last_op(void) {
    return g_last_op;
}

extern "C" int64_t ori_gizmo_is_full_build(void) {
#if defined(ORI_IMGUIZMO_FULL)
    return 1;
#else
    return 0;
#endif
}

/* Optional full gizmo when built with ORI_IMGUIZMO_FULL=1 */
#if defined(ORI_IMGUIZMO_FULL)
#include "imgui.h"
#include "../vendor/ImGuizmo.h"

static float g_view[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
static float g_proj[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

static int64_t manipulate_op(ImGuizmo::OPERATION op, int64_t book_op) {
    if (ImGui::GetCurrentContext() == nullptr) {
        g_last_op = book_op;
        return 0;
    }
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();
    bool changed = ImGuizmo::Manipulate(
        g_view, g_proj, op, ImGuizmo::WORLD, g_matrix);
    if (changed) {
        sync_components_from_matrix();
    }
    g_last_op = book_op;
    return changed ? 1 : 0;
}

extern "C" int64_t ori_gizmo_manipulate_translate(void) {
    return manipulate_op(ImGuizmo::TRANSLATE, 1);
}

extern "C" int64_t ori_gizmo_manipulate_rotate(void) {
    return manipulate_op(ImGuizmo::ROTATE, 2);
}

extern "C" int64_t ori_gizmo_manipulate_scale(void) {
    return manipulate_op(ImGuizmo::SCALE, 3);
}

extern "C" void ori_gizmo_set_view_m(int64_t index, int64_t value_m) {
    if (index < 0 || index > 15) return;
    g_view[(int)index] = f_from_m(value_m);
}

extern "C" void ori_gizmo_set_proj_m(int64_t index, int64_t value_m) {
    if (index < 0 || index > 15) return;
    g_proj[(int)index] = f_from_m(value_m);
}
#else
extern "C" int64_t ori_gizmo_manipulate_translate(void) {
    g_last_op = 1;
    return 0;
}

extern "C" int64_t ori_gizmo_manipulate_rotate(void) {
    g_last_op = 2;
    return 0;
}

extern "C" int64_t ori_gizmo_manipulate_scale(void) {
    g_last_op = 3;
    return 0;
}

extern "C" void ori_gizmo_set_view_m(int64_t /*index*/, int64_t /*value_m*/) {}
extern "C" void ori_gizmo_set_proj_m(int64_t /*index*/, int64_t /*value_m*/) {}
#endif
