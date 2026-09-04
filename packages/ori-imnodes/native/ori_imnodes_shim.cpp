/* imnodes: headless graph bookkeeping always; full editor when ORI_IMNODES_FULL=1. */
#include <stdint.h>

#define MAX_NODES 64
#define MAX_PINS 128
#define MAX_LINKS 128

static int g_nodes[MAX_NODES];
static int g_node_n = 0;

/* pin kind: 0 = input, 1 = output */
static int g_pin_ids[MAX_PINS];
static int g_pin_kinds[MAX_PINS];
static int g_pin_n = 0;

static int g_links_id[MAX_LINKS];
static int g_links_a[MAX_LINKS];
static int g_links_b[MAX_LINKS];
static int g_link_n = 0;

extern "C" void ori_imnodes_graph_clear(void) {
    g_node_n = 0;
    g_pin_n = 0;
    g_link_n = 0;
}

extern "C" int64_t ori_imnodes_graph_add_node(int64_t id) {
    for (int i = 0; i < g_node_n; ++i) {
        if (g_nodes[i] == (int)id) return 0; /* idempotent even at capacity */
    }
    if (g_node_n >= MAX_NODES) return 1;
    g_nodes[g_node_n++] = (int)id;
    return 0;
}

extern "C" int64_t ori_imnodes_graph_add_pin(int64_t id, int64_t kind) {
    for (int i = 0; i < g_pin_n; ++i) {
        if (g_pin_ids[i] == (int)id) {
            g_pin_kinds[i] = (int)kind;
            return 0;
        }
    }
    if (g_pin_n >= MAX_PINS) return 1;
    g_pin_ids[g_pin_n] = (int)id;
    g_pin_kinds[g_pin_n] = (int)kind;
    g_pin_n++;
    return 0;
}

extern "C" int64_t ori_imnodes_graph_add_link_id(int64_t link_id, int64_t a, int64_t b) {
    if (g_link_n >= MAX_LINKS) return 1;
    g_links_id[g_link_n] = (int)link_id;
    g_links_a[g_link_n] = (int)a;
    g_links_b[g_link_n] = (int)b;
    g_link_n++;
    return 0;
}

extern "C" int64_t ori_imnodes_graph_add_link(int64_t a, int64_t b) {
    return ori_imnodes_graph_add_link_id(0, a, b);
}

extern "C" int64_t ori_imnodes_graph_node_count(void) { return g_node_n; }
extern "C" int64_t ori_imnodes_graph_pin_count(void) { return g_pin_n; }
extern "C" int64_t ori_imnodes_graph_link_count(void) { return g_link_n; }

extern "C" int64_t ori_imnodes_graph_has_node(int64_t id) {
    for (int i = 0; i < g_node_n; ++i) {
        if (g_nodes[i] == (int)id) return 1;
    }
    return 0;
}

extern "C" int64_t ori_imnodes_graph_pin_kind(int64_t index) {
    if (index < 0 || index >= g_pin_n) return -1;
    return g_pin_kinds[(int)index];
}

extern "C" int64_t ori_imnodes_graph_pin_id(int64_t index) {
    if (index < 0 || index >= g_pin_n) return 0;
    return g_pin_ids[(int)index];
}

extern "C" int64_t ori_imnodes_graph_link_id(int64_t index) {
    if (index < 0 || index >= g_link_n) return 0;
    return g_links_id[(int)index];
}

extern "C" int64_t ori_imnodes_graph_link_start(int64_t index) {
    if (index < 0 || index >= g_link_n) return 0;
    return g_links_a[(int)index];
}

extern "C" int64_t ori_imnodes_graph_link_end(int64_t index) {
    if (index < 0 || index >= g_link_n) return 0;
    return g_links_b[(int)index];
}

#if defined(ORI_IMNODES_FULL)
#include "imgui.h"
#include "../vendor/imnodes.h"

extern "C" void ori_imnodes_create_context(void) {
    if (!ImNodes::GetCurrentContext()) ImNodes::CreateContext();
}
extern "C" void ori_imnodes_destroy_context(void) {
    if (ImNodes::GetCurrentContext()) ImNodes::DestroyContext();
}
extern "C" void ori_imnodes_begin_node_editor(void) { ImNodes::BeginNodeEditor(); }
extern "C" void ori_imnodes_end_node_editor(void) { ImNodes::EndNodeEditor(); }
extern "C" void ori_imnodes_begin_node(int64_t id) { ImNodes::BeginNode((int)id); }
extern "C" void ori_imnodes_end_node(void) { ImNodes::EndNode(); }
extern "C" void ori_imnodes_begin_input_attr(int64_t id) {
    ImNodes::BeginInputAttribute((int)id);
}
extern "C" void ori_imnodes_begin_output_attr(int64_t id) {
    ImNodes::BeginOutputAttribute((int)id);
}
extern "C" void ori_imnodes_end_attr(void) { ImNodes::EndInputAttribute(); }
extern "C" void ori_imnodes_link(int64_t id, int64_t start_attr, int64_t end_attr) {
    ImNodes::Link((int)id, (int)start_attr, (int)end_attr);
}
#else
/* Headless stubs: no-ops for ImGui frame; bookkeeping stays on graph_* APIs. */
extern "C" void ori_imnodes_create_context(void) {}
extern "C" void ori_imnodes_destroy_context(void) {}
extern "C" void ori_imnodes_begin_node_editor(void) {}
extern "C" void ori_imnodes_end_node_editor(void) {}
extern "C" void ori_imnodes_begin_node(int64_t id) { (void)id; }
extern "C" void ori_imnodes_end_node(void) {}
extern "C" void ori_imnodes_begin_input_attr(int64_t id) { (void)id; }
extern "C" void ori_imnodes_begin_output_attr(int64_t id) { (void)id; }
extern "C" void ori_imnodes_end_attr(void) {}
extern "C" void ori_imnodes_link(int64_t id, int64_t a, int64_t b) {
    (void)id;
    (void)a;
    (void)b;
}
#endif
