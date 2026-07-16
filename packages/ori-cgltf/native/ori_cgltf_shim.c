/* cgltf bindings for Ori — int64 ABI for pointers and counts.
 * Milli-float: physical value * 1000 as int64 (TRS, colors, mesh samples).
 * Export layout (interleaved floats, raylib-friendly):
 *   [px, py, pz, nx, ny, nz] * vertex_count
 * Indices: int64_t triangle soup (0-based). */
#define CGLTF_IMPLEMENTATION
#include "../vendor/cgltf.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static cgltf_data *g_data = NULL;

/* Last export_mesh result (owned by shim; freed on free/reload/re-export). */
static float *g_export_floats = NULL;
static int64_t *g_export_indices = NULL;
static cgltf_size g_export_vert_count = 0;
static cgltf_size g_export_index_count = 0;
static cgltf_size g_export_float_count = 0;

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

static const cgltf_mesh *mesh_at(int64_t mesh_i) {
    if (!g_data || mesh_i < 0 || (cgltf_size)mesh_i >= g_data->meshes_count) {
        return NULL;
    }
    return &g_data->meshes[mesh_i];
}

static const cgltf_primitive *prim_at(int64_t mesh_i, int64_t prim_i) {
    const cgltf_mesh *mesh = mesh_at(mesh_i);
    if (!mesh || prim_i < 0 || (cgltf_size)prim_i >= mesh->primitives_count) {
        return NULL;
    }
    return &mesh->primitives[prim_i];
}

static const cgltf_accessor *attr_accessor(
    const cgltf_primitive *prim,
    cgltf_attribute_type type
) {
    if (!prim) return NULL;
    for (cgltf_size i = 0; i < prim->attributes_count; ++i) {
        if (prim->attributes[i].type == type && prim->attributes[i].data) {
            return prim->attributes[i].data;
        }
    }
    return NULL;
}

static const cgltf_accessor *position_accessor(const cgltf_primitive *prim) {
    const cgltf_accessor *pos = attr_accessor(prim, cgltf_attribute_type_position);
    if (pos) return pos;
    if (prim && prim->attributes_count > 0) return prim->attributes[0].data;
    return NULL;
}

/* 0 = success, 1 = failure. Replaces any previously loaded document. */
int64_t ori_cgltf_load_file(int64_t path_ptr) {
    free_export();
    if (g_data) {
        cgltf_free(g_data);
        g_data = NULL;
    }
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path || !path[0]) return 1;

    cgltf_options options;
    memset(&options, 0, sizeof(options));
    cgltf_data *data = NULL;
    cgltf_result r = cgltf_parse_file(&options, path, &data);
    if (r != cgltf_result_success || !data) {
        return 1;
    }
    /* Load buffers when present (embedded base64 / external); ignore failure
       so metadata-only glTF still works for counts. */
    (void)cgltf_load_buffers(&options, data, path);
    g_data = data;
    return 0;
}

void ori_cgltf_free(void) {
    free_export();
    if (g_data) {
        cgltf_free(g_data);
        g_data = NULL;
    }
}

int64_t ori_cgltf_mesh_count(void) {
    return g_data ? (int64_t)g_data->meshes_count : 0;
}

int64_t ori_cgltf_node_count(void) {
    return g_data ? (int64_t)g_data->nodes_count : 0;
}

int64_t ori_cgltf_material_count(void) {
    return g_data ? (int64_t)g_data->materials_count : 0;
}

int64_t ori_cgltf_animation_count(void) {
    return g_data ? (int64_t)g_data->animations_count : 0;
}

/* Vertex count of mesh 0: POSITION accessor on first primitive, else first attr. */
int64_t ori_cgltf_mesh0_vertex_count(void) {
    const cgltf_primitive *prim = prim_at(0, 0);
    const cgltf_accessor *acc = position_accessor(prim);
    return acc ? (int64_t)acc->count : 0;
}

int64_t ori_cgltf_mesh_primitive_count(int64_t mesh_i) {
    const cgltf_mesh *mesh = mesh_at(mesh_i);
    return mesh ? (int64_t)mesh->primitives_count : 0;
}

int64_t ori_cgltf_mesh_vertex_count(int64_t mesh_i, int64_t prim_i) {
    const cgltf_accessor *acc = position_accessor(prim_at(mesh_i, prim_i));
    return acc ? (int64_t)acc->count : 0;
}

int64_t ori_cgltf_mesh_index_count(int64_t mesh_i, int64_t prim_i) {
    const cgltf_primitive *prim = prim_at(mesh_i, prim_i);
    if (!prim) return 0;
    if (prim->indices) return (int64_t)prim->indices->count;
    const cgltf_accessor *pos = position_accessor(prim);
    return pos ? (int64_t)pos->count : 0;
}

int64_t ori_cgltf_mesh_has_position(int64_t mesh_i, int64_t prim_i) {
    return attr_accessor(prim_at(mesh_i, prim_i), cgltf_attribute_type_position) ? 1 : 0;
}

int64_t ori_cgltf_mesh_has_normal(int64_t mesh_i, int64_t prim_i) {
    return attr_accessor(prim_at(mesh_i, prim_i), cgltf_attribute_type_normal) ? 1 : 0;
}

int64_t ori_cgltf_node_has_translation(int64_t node_i) {
    if (!g_data || node_i < 0 || (cgltf_size)node_i >= g_data->nodes_count) return 0;
    return g_data->nodes[node_i].has_translation ? 1 : 0;
}

int64_t ori_cgltf_node_has_rotation(int64_t node_i) {
    if (!g_data || node_i < 0 || (cgltf_size)node_i >= g_data->nodes_count) return 0;
    return g_data->nodes[node_i].has_rotation ? 1 : 0;
}

int64_t ori_cgltf_node_has_scale(int64_t node_i) {
    if (!g_data || node_i < 0 || (cgltf_size)node_i >= g_data->nodes_count) return 0;
    return g_data->nodes[node_i].has_scale ? 1 : 0;
}

/* TRS milli. Missing translation → 0; missing rotation → identity (w=1000);
 * missing scale → 1000. axis: 0=x,1=y,2=z; rotation component: 0=x,1=y,2=z,3=w. */
int64_t ori_cgltf_node_translation_m(int64_t node_i, int64_t axis) {
    if (!g_data || node_i < 0 || (cgltf_size)node_i >= g_data->nodes_count) return 0;
    if (axis < 0 || axis > 2) return 0;
    const cgltf_node *n = &g_data->nodes[node_i];
    if (!n->has_translation) return 0;
    return milli(n->translation[axis]);
}

int64_t ori_cgltf_node_rotation_m(int64_t node_i, int64_t component) {
    if (!g_data || node_i < 0 || (cgltf_size)node_i >= g_data->nodes_count) return 0;
    if (component < 0 || component > 3) return 0;
    const cgltf_node *n = &g_data->nodes[node_i];
    if (!n->has_rotation) {
        return component == 3 ? 1000 : 0;
    }
    return milli(n->rotation[component]);
}

int64_t ori_cgltf_node_scale_m(int64_t node_i, int64_t axis) {
    if (!g_data || node_i < 0 || (cgltf_size)node_i >= g_data->nodes_count) return 0;
    if (axis < 0 || axis > 2) return 0;
    const cgltf_node *n = &g_data->nodes[node_i];
    if (!n->has_scale) return 1000;
    return milli(n->scale[axis]);
}

/* rgba channel 0..3; default base color is white (1000 each) when material exists. */
int64_t ori_cgltf_material_base_color_m(int64_t mat_i, int64_t channel) {
    if (!g_data || mat_i < 0 || (cgltf_size)mat_i >= g_data->materials_count) {
        return 0;
    }
    if (channel < 0 || channel > 3) return 0;
    const cgltf_material *m = &g_data->materials[mat_i];
    return milli(m->pbr_metallic_roughness.base_color_factor[channel]);
}

/* Build interleaved float mesh buffer for mesh/prim.
 * Layout: [px,py,pz, nx,ny,nz] * verts. Missing NORMAL → (0,1,0).
 * Missing indices → sequential 0..vert_count-1.
 * Returns 0 on success, 1 on failure. */
int64_t ori_cgltf_export_mesh(int64_t mesh_i, int64_t prim_i) {
    free_export();
    const cgltf_primitive *prim = prim_at(mesh_i, prim_i);
    if (!prim) return 1;

    const cgltf_accessor *pos_acc =
        attr_accessor(prim, cgltf_attribute_type_position);
    if (!pos_acc || pos_acc->count == 0) return 1;

    const cgltf_accessor *nrm_acc =
        attr_accessor(prim, cgltf_attribute_type_normal);

    cgltf_size vcount = pos_acc->count;
    cgltf_size fcount = vcount * 6;
    float *floats = (float *)malloc(fcount * sizeof(float));
    if (!floats) return 1;

    for (cgltf_size v = 0; v < vcount; ++v) {
        float p[3] = {0.0f, 0.0f, 0.0f};
        float n[3] = {0.0f, 1.0f, 0.0f};
        (void)cgltf_accessor_read_float(pos_acc, v, p, 3);
        if (nrm_acc) {
            (void)cgltf_accessor_read_float(nrm_acc, v, n, 3);
        }
        floats[v * 6 + 0] = p[0];
        floats[v * 6 + 1] = p[1];
        floats[v * 6 + 2] = p[2];
        floats[v * 6 + 3] = n[0];
        floats[v * 6 + 4] = n[1];
        floats[v * 6 + 5] = n[2];
    }

    cgltf_size icount = 0;
    int64_t *indices = NULL;
    if (prim->indices && prim->indices->count > 0) {
        icount = prim->indices->count;
        indices = (int64_t *)malloc(icount * sizeof(int64_t));
        if (!indices) {
            free(floats);
            return 1;
        }
        for (cgltf_size i = 0; i < icount; ++i) {
            indices[i] = (int64_t)cgltf_accessor_read_index(prim->indices, i);
        }
    } else {
        icount = vcount;
        indices = (int64_t *)malloc(icount * sizeof(int64_t));
        if (!indices) {
            free(floats);
            return 1;
        }
        for (cgltf_size i = 0; i < icount; ++i) {
            indices[i] = (int64_t)i;
        }
    }

    g_export_floats = floats;
    g_export_indices = indices;
    g_export_vert_count = vcount;
    g_export_index_count = icount;
    g_export_float_count = fcount;
    return 0;
}

int64_t ori_cgltf_export_vertex_count(void) {
    return (int64_t)g_export_vert_count;
}

int64_t ori_cgltf_export_index_count(void) {
    return (int64_t)g_export_index_count;
}

int64_t ori_cgltf_export_float_count(void) {
    return (int64_t)g_export_float_count;
}

/* Milli sample of exported interleaved float[i]. */
int64_t ori_cgltf_export_float_m(int64_t i) {
    if (!g_export_floats || i < 0 || (cgltf_size)i >= g_export_float_count) {
        return 0;
    }
    return milli(g_export_floats[i]);
}

int64_t ori_cgltf_export_index(int64_t i) {
    if (!g_export_indices || i < 0 || (cgltf_size)i >= g_export_index_count) {
        return -1;
    }
    return g_export_indices[i];
}

/* Convenience: position / normal milli from last export. */
int64_t ori_cgltf_export_pos_m(int64_t vert_i, int64_t axis) {
    if (axis < 0 || axis > 2) return 0;
    return ori_cgltf_export_float_m(vert_i * 6 + axis);
}

int64_t ori_cgltf_export_normal_m(int64_t vert_i, int64_t axis) {
    if (axis < 0 || axis > 2) return 0;
    return ori_cgltf_export_float_m(vert_i * 6 + 3 + axis);
}

/* Raw pointers for native upload helpers (raylib etc.). */
int64_t ori_cgltf_export_floats_ptr(void) {
    return (int64_t)(intptr_t)g_export_floats;
}

int64_t ori_cgltf_export_indices_ptr(void) {
    return (int64_t)(intptr_t)g_export_indices;
}

/* Checksum of milli-rounded floats (stable test fingerprint). */
int64_t ori_cgltf_export_sum_m(void) {
    int64_t s = 0;
    for (cgltf_size i = 0; i < g_export_float_count; ++i) {
        s += milli(g_export_floats[i]);
    }
    return s;
}
