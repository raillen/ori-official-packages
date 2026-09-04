/* fast_obj bindings for Ori — int64 ABI for pointers and counts.
 * Milli-float: physical value * 1000 as int64.
 *
 * Flatten export layout (interleaved floats, raylib-friendly):
 *   [px, py, pz, nx, ny, nz, u, v] * vertex_count
 * vertex_count = triangle-corner count after fan triangulation of faces.
 * Indices: int64_t triangle list (0-based sequential corners).
 */
#define FAST_OBJ_IMPLEMENTATION
#include "../vendor/fast_obj.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static fastObjMesh *g_mesh = NULL;

/* Last flatten/export result (owned by shim; freed on free/reload/re-export). */
static float *g_export_floats = NULL;
static int64_t *g_export_indices = NULL;
static size_t g_export_vert_count = 0;
static size_t g_export_index_count = 0;
static size_t g_export_float_count = 0;

static void free_export(void) {
    free(g_export_floats);
    free(g_export_indices);
    g_export_floats = NULL;
    g_export_indices = NULL;
    g_export_vert_count = 0;
    g_export_index_count = 0;
    g_export_float_count = 0;
}

static int64_t milli(float v) {
    return (int64_t)(v * 1000.0f);
}

int64_t ori_fast_obj_load(int64_t path_ptr) {
    free_export();
    if (g_mesh) {
        fast_obj_destroy(g_mesh);
        g_mesh = NULL;
    }
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path || !path[0]) return 1;
    g_mesh = fast_obj_read(path);
    return g_mesh ? 0 : 1;
}

void ori_fast_obj_free(void) {
    free_export();
    if (g_mesh) {
        fast_obj_destroy(g_mesh);
        g_mesh = NULL;
    }
}

/* Counts include the dummy zero entry at index 0 (fast_obj convention). */
int64_t ori_fast_obj_position_count(void) {
    return g_mesh ? (int64_t)g_mesh->position_count : 0;
}

int64_t ori_fast_obj_face_count(void) {
    return g_mesh ? (int64_t)g_mesh->face_count : 0;
}

int64_t ori_fast_obj_normal_count(void) {
    return g_mesh ? (int64_t)g_mesh->normal_count : 0;
}

int64_t ori_fast_obj_texcoord_count(void) {
    return g_mesh ? (int64_t)g_mesh->texcoord_count : 0;
}

int64_t ori_fast_obj_index_count(void) {
    return g_mesh ? (int64_t)g_mesh->index_count : 0;
}

int64_t ori_fast_obj_material_count(void) {
    return g_mesh ? (int64_t)g_mesh->material_count : 0;
}

/* Length of material name string (excluding NUL). Missing → 0. */
int64_t ori_fast_obj_material_name_len(int64_t mat_i) {
    if (!g_mesh || mat_i < 0 || (unsigned int)mat_i >= g_mesh->material_count) {
        return 0;
    }
    const char *name = g_mesh->materials[mat_i].name;
    if (!name) return 0;
    return (int64_t)strlen(name);
}

/* Byte of material name at offset (0..len-1). Out of range → 0. */
int64_t ori_fast_obj_material_name_byte(int64_t mat_i, int64_t offset) {
    if (!g_mesh || mat_i < 0 || (unsigned int)mat_i >= g_mesh->material_count) {
        return 0;
    }
    const char *name = g_mesh->materials[mat_i].name;
    if (!name || offset < 0) return 0;
    size_t len = strlen(name);
    if ((size_t)offset >= len) return 0;
    return (int64_t)(unsigned char)name[offset];
}

/* Sample position components (milli) for vertex index (0-based among real verts:
   mesh index i+1 because positions[0] is dummy). */
int64_t ori_fast_obj_pos_m(int64_t vertex_index, int64_t axis) {
    if (!g_mesh || axis < 0 || axis > 2) return 0;
    unsigned int i = (unsigned int)(vertex_index + 1);
    if (i >= g_mesh->position_count) return 0;
    float v = g_mesh->positions[3 * i + (unsigned int)axis];
    return milli(v);
}

/* 0-based among real normals (skip dummy at 0). axis 0..2. */
int64_t ori_fast_obj_normal_m(int64_t normal_index, int64_t axis) {
    if (!g_mesh || axis < 0 || axis > 2) return 0;
    unsigned int i = (unsigned int)(normal_index + 1);
    if (i >= g_mesh->normal_count) return 0;
    float v = g_mesh->normals[3 * i + (unsigned int)axis];
    return milli(v);
}

/* 0-based among real texcoords (skip dummy at 0). axis 0=u, 1=v. */
int64_t ori_fast_obj_texcoord_m(int64_t texcoord_index, int64_t axis) {
    if (!g_mesh || axis < 0 || axis > 1) return 0;
    unsigned int i = (unsigned int)(texcoord_index + 1);
    if (i >= g_mesh->texcoord_count) return 0;
    float v = g_mesh->texcoords[2 * i + (unsigned int)axis];
    return milli(v);
}

/* Face-corner index arrays (fast_obj: p/t/n are 1-based into attribute arrays;
 * 0 means attribute missing). Return stored values as int64. */
int64_t ori_fast_obj_index_p(int64_t i) {
    if (!g_mesh || i < 0 || (unsigned int)i >= g_mesh->index_count) return 0;
    return (int64_t)g_mesh->indices[i].p;
}

int64_t ori_fast_obj_index_t(int64_t i) {
    if (!g_mesh || i < 0 || (unsigned int)i >= g_mesh->index_count) return 0;
    return (int64_t)g_mesh->indices[i].t;
}

int64_t ori_fast_obj_index_n(int64_t i) {
    if (!g_mesh || i < 0 || (unsigned int)i >= g_mesh->index_count) return 0;
    return (int64_t)g_mesh->indices[i].n;
}

/* Count triangle corners after fan triangulation of all faces. */
static size_t triangle_corner_count(void) {
    if (!g_mesh) return 0;
    size_t corners = 0;
    for (unsigned int f = 0; f < g_mesh->face_count; ++f) {
        unsigned int fv = g_mesh->face_vertices[f];
        if (fv >= 3) {
            corners += (size_t)(fv - 2) * 3;
        }
    }
    return corners;
}

/* Flatten mesh: fan-triangulate faces; one unique vertex per triangle corner.
 * Layout: [px,py,pz, nx,ny,nz, u,v] * V.
 * Missing normal → (0,0,1); missing texcoord → (0,0).
 * Returns 0 on success, 1 on failure. */
int64_t ori_fast_obj_export_mesh(void) {
    free_export();
    if (!g_mesh || g_mesh->face_count == 0 || g_mesh->index_count == 0) {
        return 1;
    }

    size_t vcount = triangle_corner_count();
    if (vcount == 0) return 1;

    size_t fcount = vcount * 8;
    float *floats = (float *)malloc(fcount * sizeof(float));
    int64_t *indices = (int64_t *)malloc(vcount * sizeof(int64_t));
    if (!floats || !indices) {
        free(floats);
        free(indices);
        return 1;
    }

    size_t out_v = 0;
    unsigned int index_offset = 0;
    for (unsigned int f = 0; f < g_mesh->face_count; ++f) {
        unsigned int fv = g_mesh->face_vertices[f];
        if (fv < 3) {
            index_offset += fv;
            continue;
        }
        /* Fan: (0, i, i+1) for i = 1..fv-2 */
        for (unsigned int t = 1; t + 1 < fv; ++t) {
            unsigned int corners[3] = {
                index_offset + 0,
                index_offset + t,
                index_offset + t + 1
            };
            for (int c = 0; c < 3; ++c) {
                unsigned int ii = corners[c];
                if (ii >= g_mesh->index_count) {
                    free(floats);
                    free(indices);
                    return 1;
                }
                fastObjIndex idx = g_mesh->indices[ii];

                float px = 0.0f, py = 0.0f, pz = 0.0f;
                if (idx.p > 0 && idx.p < g_mesh->position_count) {
                    unsigned int p = idx.p;
                    px = g_mesh->positions[3 * p + 0];
                    py = g_mesh->positions[3 * p + 1];
                    pz = g_mesh->positions[3 * p + 2];
                }

                float nx = 0.0f, ny = 0.0f, nz = 1.0f;
                if (idx.n > 0 && idx.n < g_mesh->normal_count) {
                    unsigned int n = idx.n;
                    nx = g_mesh->normals[3 * n + 0];
                    ny = g_mesh->normals[3 * n + 1];
                    nz = g_mesh->normals[3 * n + 2];
                }

                float u = 0.0f, v = 0.0f;
                if (idx.t > 0 && idx.t < g_mesh->texcoord_count) {
                    unsigned int tt = idx.t;
                    u = g_mesh->texcoords[2 * tt + 0];
                    v = g_mesh->texcoords[2 * tt + 1];
                }

                size_t base = out_v * 8;
                floats[base + 0] = px;
                floats[base + 1] = py;
                floats[base + 2] = pz;
                floats[base + 3] = nx;
                floats[base + 4] = ny;
                floats[base + 5] = nz;
                floats[base + 6] = u;
                floats[base + 7] = v;
                indices[out_v] = (int64_t)out_v;
                out_v++;
            }
        }
        index_offset += fv;
    }

    if (out_v != vcount) {
        free(floats);
        free(indices);
        return 1;
    }

    g_export_floats = floats;
    g_export_indices = indices;
    g_export_vert_count = vcount;
    g_export_index_count = vcount;
    g_export_float_count = fcount;
    return 0;
}

int64_t ori_fast_obj_export_vertex_count(void) {
    return (int64_t)g_export_vert_count;
}

int64_t ori_fast_obj_export_index_count(void) {
    return (int64_t)g_export_index_count;
}

int64_t ori_fast_obj_export_float_count(void) {
    return (int64_t)g_export_float_count;
}

int64_t ori_fast_obj_export_float_m(int64_t i) {
    if (!g_export_floats || i < 0 || (size_t)i >= g_export_float_count) {
        return 0;
    }
    return milli(g_export_floats[i]);
}

int64_t ori_fast_obj_export_index(int64_t i) {
    if (!g_export_indices || i < 0 || (size_t)i >= g_export_index_count) {
        return -1;
    }
    return g_export_indices[i];
}

int64_t ori_fast_obj_export_pos_m(int64_t vert_i, int64_t axis) {
    if (axis < 0 || axis > 2) return 0;
    return ori_fast_obj_export_float_m(vert_i * 8 + axis);
}

int64_t ori_fast_obj_export_normal_m(int64_t vert_i, int64_t axis) {
    if (axis < 0 || axis > 2) return 0;
    return ori_fast_obj_export_float_m(vert_i * 8 + 3 + axis);
}

int64_t ori_fast_obj_export_uv_m(int64_t vert_i, int64_t axis) {
    if (axis < 0 || axis > 1) return 0;
    return ori_fast_obj_export_float_m(vert_i * 8 + 6 + axis);
}

int64_t ori_fast_obj_export_floats_ptr(void) {
    return (int64_t)(intptr_t)g_export_floats;
}

int64_t ori_fast_obj_export_indices_ptr(void) {
    return (int64_t)(intptr_t)g_export_indices;
}

/* Checksum of milli-rounded floats (stable test fingerprint). */
int64_t ori_fast_obj_export_sum_m(void) {
    int64_t s = 0;
    for (size_t i = 0; i < g_export_float_count; ++i) {
        s += milli(g_export_floats[i]);
    }
    return s;
}
