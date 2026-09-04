/* Native file dialogs via portable-file-dialogs (header-only C++).
 * int64_t ABI for Ori. Headless: NFD_SMOKE_SKIP_UI=1 skips real GUI. */
#include "../vendor/portable-file-dialogs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

/* Primary path (first selection) — null-terminated. */
static char g_path[4096];
static int g_ok = 0;

/* Multi-open: up to MAX_PATHS stored with lengths; path 0 == g_path. */
#define MAX_PATHS 32
#define PATH_CAP 4096
static char g_paths[MAX_PATHS][PATH_CAP];
static int g_path_lens[MAX_PATHS];
static int g_path_count = 0;

static int skip_ui(void) {
    const char *e = getenv("NFD_SMOKE_SKIP_UI");
    return e && e[0] && e[0] != '0';
}

static void clear_all(void) {
    g_ok = 0;
    g_path[0] = '\0';
    g_path_count = 0;
    for (int i = 0; i < MAX_PATHS; i++) {
        g_paths[i][0] = '\0';
        g_path_lens[i] = 0;
    }
}

static void store_one(const std::string &s) {
    clear_all();
    if (s.empty()) {
        return;
    }
    size_t n = s.size();
    if (n >= PATH_CAP) {
        n = PATH_CAP - 1;
    }
    memcpy(g_path, s.c_str(), n);
    g_path[n] = '\0';
    memcpy(g_paths[0], g_path, n + 1);
    g_path_lens[0] = (int)n;
    g_path_count = 1;
    g_ok = 1;
}

static void store_many(const std::vector<std::string> &sel) {
    clear_all();
    if (sel.empty()) {
        return;
    }
    int npaths = (int)sel.size();
    if (npaths > MAX_PATHS) {
        npaths = MAX_PATHS;
    }
    for (int i = 0; i < npaths; i++) {
        size_t n = sel[(size_t)i].size();
        if (n >= PATH_CAP) {
            n = PATH_CAP - 1;
        }
        memcpy(g_paths[i], sel[(size_t)i].c_str(), n);
        g_paths[i][n] = '\0';
        g_path_lens[i] = (int)n;
    }
    g_path_count = npaths;
    /* Primary path is first selection. */
    memcpy(g_path, g_paths[0], (size_t)g_path_lens[0] + 1);
    g_ok = 1;
}

static int64_t cancelled(void) {
    clear_all();
    return 1;
}

extern "C" int64_t ori_nfd_open_file(int64_t title_ptr, int64_t filter_ptr) {
    if (skip_ui()) {
        return cancelled();
    }
    const char *title = title_ptr ? (const char *)(intptr_t)title_ptr : "Open";
    const char *filter = filter_ptr ? (const char *)(intptr_t)filter_ptr : "";
    std::vector<std::string> filters;
    if (filter && filter[0]) {
        filters.push_back("Files");
        filters.push_back(filter);
    }
    auto sel = pfd::open_file(title, ".", filters).result();
    if (sel.empty()) {
        return cancelled();
    }
    store_one(sel[0]);
    return g_ok ? 0 : 1;
}

/* Multi-select open. Paths indexed via path_count / path_at_*. */
extern "C" int64_t ori_nfd_open_files(int64_t title_ptr, int64_t filter_ptr) {
    if (skip_ui()) {
        return cancelled();
    }
    const char *title = title_ptr ? (const char *)(intptr_t)title_ptr : "Open";
    const char *filter = filter_ptr ? (const char *)(intptr_t)filter_ptr : "";
    std::vector<std::string> filters;
    if (filter && filter[0]) {
        filters.push_back("Files");
        filters.push_back(filter);
    }
    auto sel = pfd::open_file(title, ".", filters, pfd::opt::multiselect).result();
    if (sel.empty()) {
        return cancelled();
    }
    store_many(sel);
    return g_ok ? 0 : 1;
}

extern "C" int64_t ori_nfd_save_file(int64_t title_ptr, int64_t default_name_ptr) {
    if (skip_ui()) {
        return cancelled();
    }
    const char *title = title_ptr ? (const char *)(intptr_t)title_ptr : "Save";
    const char *def = default_name_ptr ? (const char *)(intptr_t)default_name_ptr : "out.txt";
    auto path = pfd::save_file(title, def).result();
    if (path.empty()) {
        return cancelled();
    }
    store_one(path);
    return g_ok ? 0 : 1;
}

extern "C" int64_t ori_nfd_pick_folder(int64_t title_ptr) {
    if (skip_ui()) {
        return cancelled();
    }
    const char *title = title_ptr ? (const char *)(intptr_t)title_ptr : "Folder";
    auto path = pfd::select_folder(title).result();
    if (path.empty()) {
        return cancelled();
    }
    store_one(path);
    return g_ok ? 0 : 1;
}

extern "C" int64_t ori_nfd_ok(void) {
    return g_ok ? 1 : 0;
}

extern "C" int64_t ori_nfd_path_ptr(void) {
    return (int64_t)(intptr_t)g_path;
}

extern "C" int64_t ori_nfd_path_len(void) {
    return (int64_t)strlen(g_path);
}

/* Byte at index in primary path (for marshalling without string-from-ptr). */
extern "C" int64_t ori_nfd_path_byte(int64_t i) {
    if (i < 0 || i >= (int64_t)strlen(g_path)) {
        return 0;
    }
    return (int64_t)(unsigned char)g_path[(size_t)i];
}

extern "C" int64_t ori_nfd_path_sum(void) {
    int64_t s = 0;
    size_t n = strlen(g_path);
    for (size_t i = 0; i < n; i++) {
        s += (unsigned char)g_path[i];
    }
    return s;
}

extern "C" int64_t ori_nfd_path_count(void) {
    return (int64_t)g_path_count;
}

extern "C" int64_t ori_nfd_path_at_len(int64_t idx) {
    if (idx < 0 || idx >= g_path_count) {
        return 0;
    }
    return (int64_t)g_path_lens[(int)idx];
}

extern "C" int64_t ori_nfd_path_at_byte(int64_t idx, int64_t i) {
    if (idx < 0 || idx >= g_path_count) {
        return 0;
    }
    int len = g_path_lens[(int)idx];
    if (i < 0 || i >= len) {
        return 0;
    }
    return (int64_t)(unsigned char)g_paths[(int)idx][(size_t)i];
}

/* Reset to cancelled / empty (no dialog). */
extern "C" int64_t ori_nfd_clear(void) {
    clear_all();
    return 0;
}

/* Non-interactive: set primary path for tests. */
extern "C" int64_t ori_nfd_set_path_for_test(int64_t path_ptr) {
    const char *p = path_ptr ? (const char *)(intptr_t)path_ptr : "";
    if (!p || !p[0]) {
        clear_all();
        return 1;
    }
    store_one(std::string(p));
    return g_ok ? 0 : 1;
}

/* Non-interactive multi-path: semicolon-separated list. */
extern "C" int64_t ori_nfd_set_paths_for_test(int64_t paths_ptr) {
    const char *p = paths_ptr ? (const char *)(intptr_t)paths_ptr : "";
    if (!p || !p[0]) {
        clear_all();
        return 1;
    }
    std::vector<std::string> sel;
    std::string cur;
    for (const char *c = p; ; c++) {
        if (*c == ';' || *c == '\0') {
            if (!cur.empty()) {
                sel.push_back(cur);
                cur.clear();
            }
            if (*c == '\0') {
                break;
            }
        } else {
            cur.push_back(*c);
        }
    }
    if (sel.empty()) {
        clear_all();
        return 1;
    }
    store_many(sel);
    return g_ok ? 0 : 1;
}
