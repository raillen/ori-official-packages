/* Tracy instrumentation for Ori.
 * Default (no ORI_TRACY_FULL): lightweight zone/frame/message/plot counters for
 * tests and smoke without Tracy GUI or TracyClient.
 * ORI_TRACY_FULL + TRACY_ENABLE: same bookkeeping + real Tracy client emit. */
#include <stdint.h>
#include <string.h>

#define MAX_DEPTH 64
#define MAX_NAME 128

static int g_depth = 0;
static char g_names[MAX_DEPTH][MAX_NAME];
static int64_t g_zone_begins = 0;
static int64_t g_zone_ends = 0;
static int64_t g_frames = 0;
static int64_t g_messages = 0;
static int64_t g_plots = 0;
static int64_t g_last_plot_m = 0;
static int64_t g_overflow_begins = 0;
static int64_t g_orphan_ends = 0;

#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
#include "tracy/TracyC.h"
static TracyCZoneCtx g_zone_ctx[MAX_DEPTH];
#endif

static void copy_zone_name(int idx, const char *name) {
    size_t n = strlen(name);
    if (n >= MAX_NAME) n = MAX_NAME - 1;
    memcpy(g_names[idx], name, n);
    g_names[idx][n] = '\0';
}

extern "C" void ori_tracy_zone_begin(int64_t name_ptr) {
    const char *name = name_ptr ? (const char *)(intptr_t)name_ptr : "zone";
    size_t name_len = strlen(name);

#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
    if (g_depth < MAX_DEPTH) {
        uint64_t srcloc = ___tracy_alloc_srcloc_name(
            0,
            "ori",
            3,
            "zone",
            4,
            name,
            name_len,
            0);
        g_zone_ctx[g_depth] = ___tracy_emit_zone_begin_alloc(srcloc, 1);
    }
#endif

    if (g_depth < MAX_DEPTH) {
        copy_zone_name(g_depth, name);
        g_depth++;
    } else {
        g_overflow_begins++;
    }
    g_zone_begins++;
}

extern "C" void ori_tracy_zone_end(void) {
    if (g_depth > 0) {
#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
        ___tracy_emit_zone_end(g_zone_ctx[g_depth - 1]);
#endif
        g_depth--;
    } else {
        g_orphan_ends++;
    }
    g_zone_ends++;
}

extern "C" int64_t ori_tracy_zone_depth(void) { return g_depth; }
extern "C" int64_t ori_tracy_zone_begins(void) { return g_zone_begins; }
extern "C" int64_t ori_tracy_zone_ends(void) { return g_zone_ends; }
extern "C" int64_t ori_tracy_overflow_begins(void) { return g_overflow_begins; }
extern "C" int64_t ori_tracy_orphan_ends(void) { return g_orphan_ends; }

extern "C" void ori_tracy_frame_mark(void) {
#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
    TracyCFrameMark;
#endif
    g_frames++;
}

extern "C" int64_t ori_tracy_frame_count(void) { return g_frames; }

extern "C" void ori_tracy_reset_stats(void) {
    /* Drop open FULL zones so counters stay consistent after reset. */
#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
    while (g_depth > 0) {
        ___tracy_emit_zone_end(g_zone_ctx[g_depth - 1]);
        g_depth--;
    }
#else
    g_depth = 0;
#endif
    g_zone_begins = 0;
    g_zone_ends = 0;
    g_frames = 0;
    g_messages = 0;
    g_plots = 0;
    g_last_plot_m = 0;
    g_overflow_begins = 0;
    g_orphan_ends = 0;
}

extern "C" int64_t ori_tracy_is_full_build(void) {
#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
    return 1;
#else
    return 0;
#endif
}

extern "C" void ori_tracy_message(int64_t text_ptr) {
    const char *t = text_ptr ? (const char *)(intptr_t)text_ptr : "";
#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
    TracyCMessage(t, strlen(t));
#endif
    g_messages++;
}

extern "C" void ori_tracy_plot_m(int64_t name_ptr, int64_t value_m) {
    const char *n = name_ptr ? (const char *)(intptr_t)name_ptr : "plot";
#if defined(ORI_TRACY_FULL) && defined(TRACY_ENABLE)
    TracyCPlot(n, (double)value_m / 1000.0);
#else
    (void)n;
#endif
    g_plots++;
    g_last_plot_m = value_m;
}

extern "C" int64_t ori_tracy_message_count(void) { return g_messages; }
extern "C" int64_t ori_tracy_plot_count(void) { return g_plots; }
extern "C" int64_t ori_tracy_last_plot_value_m(void) { return g_last_plot_m; }
