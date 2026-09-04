/* Clay layout bindings for Ori — int64 ABI; headless (no window).
 *
 * Product surface (maturity 5):
 *   - two-box LTR siblings (pad/gap)
 *   - nested multi-box (root TTB + body LTR, padding/gap)
 *   - command-list export (rects + colors) for pure-Ori / raylib drawers
 *
 * Bounds and command geometry reported in milli-pixels (value * 1000).
 * Colors reported as 0–255 channel ints.
 */
#define CLAY_IMPLEMENTATION
#include "../vendor/clay.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ORI_CLAY_MAX_BOXES 16
#define ORI_CLAY_MAX_CMDS  64

typedef struct {
    float x, y, w, h;
} OriClayBox;

typedef struct {
    int32_t type;
    float x, y, w, h;
    float r, g, b, a;
} OriClayCmd;

static void *g_memory = NULL;
static int g_inited = 0;
static int g_error = 0;
static int g_box_count = 0;
static int g_cmd_count = 0;
static OriClayBox g_boxes[ORI_CLAY_MAX_BOXES];
static OriClayCmd g_cmds[ORI_CLAY_MAX_CMDS];

static void ori_clay_handle_error(Clay_ErrorData errorData) {
    (void)errorData;
    g_error = 1;
}

static Clay_Dimensions ori_clay_measure_text(
    Clay_StringSlice text,
    Clay_TextElementConfig *config,
    void *userData
) {
    (void)userData;
    float font_size = 16.0f;
    if (config && config->fontSize > 0) {
        font_size = (float)config->fontSize;
    }
    /* Monospace stub: ~0.5 * fontSize per character. */
    return (Clay_Dimensions){
        .width = (float)text.length * font_size * 0.5f,
        .height = font_size,
    };
}

static int64_t ori_clay_f_to_milli(float v) {
    return (int64_t)(v * 1000.0f);
}

static void ori_clay_reset_frame(void) {
    g_error = 0;
    g_box_count = 0;
    g_cmd_count = 0;
    memset(g_boxes, 0, sizeof(g_boxes));
    memset(g_cmds, 0, sizeof(g_cmds));
}

static void ori_clay_store_box(float x, float y, float w, float h) {
    if (g_box_count >= ORI_CLAY_MAX_BOXES) {
        return;
    }
    g_boxes[g_box_count].x = x;
    g_boxes[g_box_count].y = y;
    g_boxes[g_box_count].w = w;
    g_boxes[g_box_count].h = h;
    g_box_count++;
}

static void ori_clay_store_element(Clay_ElementId id) {
    Clay_ElementData data = Clay_GetElementData(id);
    if (!data.found) {
        return;
    }
    ori_clay_store_box(
        data.boundingBox.x,
        data.boundingBox.y,
        data.boundingBox.width,
        data.boundingBox.height
    );
}

/* Capture Clay render commands for drawer export (rects + colors). */
static void ori_clay_store_commands(Clay_RenderCommandArray cmds) {
    g_cmd_count = 0;
    for (int32_t i = 0; i < cmds.length && g_cmd_count < ORI_CLAY_MAX_CMDS; i++) {
        Clay_RenderCommand *c = Clay_RenderCommandArray_Get(&cmds, i);
        if (!c) {
            continue;
        }
        OriClayCmd *out = &g_cmds[g_cmd_count];
        out->type = (int32_t)c->commandType;
        out->x = c->boundingBox.x;
        out->y = c->boundingBox.y;
        out->w = c->boundingBox.width;
        out->h = c->boundingBox.height;
        out->r = 0.0f;
        out->g = 0.0f;
        out->b = 0.0f;
        out->a = 0.0f;
        if (c->commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE) {
            Clay_Color col = c->renderData.rectangle.backgroundColor;
            out->r = col.r;
            out->g = col.g;
            out->b = col.b;
            out->a = col.a;
        }
        g_cmd_count++;
    }
}

/* 0 = ok, 1 = failure. Layout size in whole pixels. */
int64_t ori_clay_init(int64_t width, int64_t height) {
    if (g_inited) {
        return 0;
    }
    if (width <= 0 || height <= 0) {
        return 1;
    }
    ori_clay_reset_frame();

    uint32_t mem_size = Clay_MinMemorySize();
    g_memory = malloc(mem_size);
    if (!g_memory) {
        return 1;
    }
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(mem_size, g_memory);
    Clay_Initialize(
        arena,
        (Clay_Dimensions){ (float)width, (float)height },
        (Clay_ErrorHandler){ ori_clay_handle_error, NULL }
    );
    Clay_SetMeasureTextFunction(ori_clay_measure_text, NULL);
    if (g_error) {
        free(g_memory);
        g_memory = NULL;
        return 1;
    }
    g_inited = 1;
    return 0;
}

void ori_clay_shutdown(void) {
    if (g_memory) {
        free(g_memory);
        g_memory = NULL;
    }
    g_inited = 0;
    ori_clay_reset_frame();
}

/*
 * Begin layout, open root + two sibling boxes, end.
 * Stores bounds for BoxA and BoxB (indices 0 and 1).
 * Root: pad=16, childGap=16, LEFT_TO_RIGHT.
 * Returns element count (boxes stored), or 0 on failure.
 */
int64_t ori_clay_layout_two_boxes(void) {
    if (!g_inited) {
        return 0;
    }
    ori_clay_reset_frame();

    Clay_BeginLayout();

    CLAY(CLAY_ID("Root"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
        .backgroundColor = { 40, 40, 40, 255 },
    }) {
        CLAY(CLAY_ID("BoxA"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIXED(100),
                    .height = CLAY_SIZING_FIXED(50),
                },
            },
            .backgroundColor = { 200, 50, 50, 255 },
        }) {}
        CLAY(CLAY_ID("BoxB"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIXED(120),
                    .height = CLAY_SIZING_FIXED(50),
                },
            },
            .backgroundColor = { 50, 200, 50, 255 },
        }) {}
    }

    Clay_RenderCommandArray cmds = Clay_EndLayout(0.016f);
    ori_clay_store_commands(cmds);

    if (g_error) {
        g_box_count = 0;
        return 0;
    }

    ori_clay_store_element(CLAY_ID("BoxA"));
    ori_clay_store_element(CLAY_ID("BoxB"));
    if (g_box_count < 2) {
        g_box_count = 0;
        return 0;
    }

    return (int64_t)g_box_count;
}

/*
 * Nested multi-box layout with padding/gap:
 *
 *   Root   (grow, pad=20, gap=10, TOP_TO_BOTTOM)
 *     Header (fixed 200x40)
 *     Body   (grow w, h=100, pad=12, gap=8, LEFT_TO_RIGHT)
 *       Left  (fixed 80x50)
 *       Right (fixed 90x50)
 *
 * Box indices: 0=Header, 1=Body, 2=Left, 3=Right.
 * Returns box count (4) or 0 on failure. Also fills command list.
 */
int64_t ori_clay_layout_nested(void) {
    if (!g_inited) {
        return 0;
    }
    ori_clay_reset_frame();

    Clay_BeginLayout();

    CLAY(CLAY_ID("Root"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .padding = CLAY_PADDING_ALL(20),
            .childGap = 10,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = { 40, 40, 40, 255 },
    }) {
        CLAY(CLAY_ID("Header"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIXED(200),
                    .height = CLAY_SIZING_FIXED(40),
                },
            },
            .backgroundColor = { 200, 50, 50, 255 },
        }) {}
        CLAY(CLAY_ID("Body"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIXED(100),
                },
                .padding = CLAY_PADDING_ALL(12),
                .childGap = 8,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
            .backgroundColor = { 50, 50, 200, 255 },
        }) {
            CLAY(CLAY_ID("Left"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(80),
                        .height = CLAY_SIZING_FIXED(50),
                    },
                },
                .backgroundColor = { 50, 200, 50, 255 },
            }) {}
            CLAY(CLAY_ID("Right"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(90),
                        .height = CLAY_SIZING_FIXED(50),
                    },
                },
                .backgroundColor = { 200, 200, 50, 255 },
            }) {}
        }
    }

    Clay_RenderCommandArray cmds = Clay_EndLayout(0.016f);
    ori_clay_store_commands(cmds);

    if (g_error) {
        g_box_count = 0;
        return 0;
    }

    ori_clay_store_element(CLAY_ID("Header"));
    ori_clay_store_element(CLAY_ID("Body"));
    ori_clay_store_element(CLAY_ID("Left"));
    ori_clay_store_element(CLAY_ID("Right"));
    if (g_box_count < 4) {
        g_box_count = 0;
        return 0;
    }

    return (int64_t)g_box_count;
}

int64_t ori_clay_box_count(void) {
    return (int64_t)g_box_count;
}

int64_t ori_clay_box_x_m(int64_t index) {
    if (index < 0 || index >= g_box_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_boxes[index].x);
}

int64_t ori_clay_box_y_m(int64_t index) {
    if (index < 0 || index >= g_box_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_boxes[index].y);
}

int64_t ori_clay_box_w_m(int64_t index) {
    if (index < 0 || index >= g_box_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_boxes[index].w);
}

int64_t ori_clay_box_h_m(int64_t index) {
    if (index < 0 || index >= g_box_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_boxes[index].h);
}

/* --- Command list export (drawer surface) --- */

int64_t ori_clay_cmd_count(void) {
    return (int64_t)g_cmd_count;
}

/* Clay_RenderCommandType: 0=NONE, 1=RECTANGLE, 2=BORDER, 3=TEXT, ... */
int64_t ori_clay_cmd_type(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return (int64_t)g_cmds[index].type;
}

int64_t ori_clay_cmd_x_m(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_cmds[index].x);
}

int64_t ori_clay_cmd_y_m(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_cmds[index].y);
}

int64_t ori_clay_cmd_w_m(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_cmds[index].w);
}

int64_t ori_clay_cmd_h_m(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return ori_clay_f_to_milli(g_cmds[index].h);
}

int64_t ori_clay_cmd_r(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return (int64_t)g_cmds[index].r;
}

int64_t ori_clay_cmd_g(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return (int64_t)g_cmds[index].g;
}

int64_t ori_clay_cmd_b(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return (int64_t)g_cmds[index].b;
}

int64_t ori_clay_cmd_a(int64_t index) {
    if (index < 0 || index >= g_cmd_count) {
        return 0;
    }
    return (int64_t)g_cmds[index].a;
}

/* Count RECTANGLE commands only (type == 1) — drawer convenience. */
int64_t ori_clay_rect_cmd_count(void) {
    int64_t n = 0;
    for (int i = 0; i < g_cmd_count; i++) {
        if (g_cmds[i].type == (int32_t)CLAY_RENDER_COMMAND_TYPE_RECTANGLE) {
            n++;
        }
    }
    return n;
}
