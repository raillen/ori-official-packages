#define RRES_IMPLEMENTATION
#include "rres.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ORPK pack format (product surface):
 * magic "ORPK" (4 bytes)
 * u32 version = 1 (little-endian)
 * u32 count
 * repeated count times:
 *   u32 name_len
 *   name bytes (no trailing NUL required on disk)
 *   u32 data_len
 *   data bytes
 * Resource id (runtime) = rresComputeCRC32(name, name_len)
 */

#define MAX_PACKS 8
#define MAX_ENTRIES 64
#define MAX_ENTRY_BYTES (16 * 1024 * 1024)

typedef struct {
    char name[256];
    unsigned int id;
    unsigned char *data;
    unsigned int size;
} Entry;

typedef struct {
    int live;
    int valid; /* set after successful open/create structure check */
    Entry entries[MAX_ENTRIES];
    int count;
} Pack;

static Pack g_packs[MAX_PACKS];
static char g_last_err[256];

static void set_err(const char *msg) {
    if (!msg) {
        g_last_err[0] = '\0';
        return;
    }
    strncpy(g_last_err, msg, sizeof(g_last_err) - 1);
    g_last_err[sizeof(g_last_err) - 1] = '\0';
}

static int alloc_pack(void) {
    for (int i = 0; i < MAX_PACKS; ++i) {
        if (!g_packs[i].live) {
            memset(&g_packs[i], 0, sizeof(Pack));
            g_packs[i].live = 1;
            g_packs[i].valid = 1;
            return i;
        }
    }
    return -1;
}

/* Structural validate of an ORPK file without fully retaining data.
 * Returns 0 if valid, nonzero on failure. Optionally fills *out_count. */
static int validate_orpk_file(FILE *f, uint32_t *out_count) {
    char magic[4];
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "ORPK", 4) != 0) {
        set_err("bad magic (expected ORPK)");
        return 1;
    }
    uint32_t ver = 0, count = 0;
    if (fread(&ver, 4, 1, f) != 1 || fread(&count, 4, 1, f) != 1) {
        set_err("truncated header");
        return 1;
    }
    if (ver != 1) {
        set_err("unsupported ORPK version");
        return 1;
    }
    if (count > MAX_ENTRIES) {
        set_err("too many entries");
        return 1;
    }
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t nlen = 0, dlen = 0;
        if (fread(&nlen, 4, 1, f) != 1) {
            set_err("truncated name_len");
            return 1;
        }
        if (nlen == 0 || nlen >= 256) {
            set_err("invalid name_len");
            return 1;
        }
        if (fseek(f, (long)nlen, SEEK_CUR) != 0) {
            set_err("truncated name");
            return 1;
        }
        if (fread(&dlen, 4, 1, f) != 1) {
            set_err("truncated data_len");
            return 1;
        }
        if (dlen > MAX_ENTRY_BYTES) {
            set_err("entry too large");
            return 1;
        }
        if (dlen && fseek(f, (long)dlen, SEEK_CUR) != 0) {
            set_err("truncated data");
            return 1;
        }
    }
    /* Ensure no trailing garbage is required; extra bytes are tolerated. */
    if (out_count) *out_count = count;
    set_err(NULL);
    return 0;
}

int ori_rres_create_empty(void) {
    set_err(NULL);
    return alloc_pack();
}

void ori_rres_close(int pack) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return;
    for (int i = 0; i < g_packs[pack].count; ++i) {
        free(g_packs[pack].entries[i].data);
        g_packs[pack].entries[i].data = NULL;
    }
    g_packs[pack].live = 0;
    g_packs[pack].valid = 0;
    g_packs[pack].count = 0;
}

int ori_rres_add_file(int pack, int name_ptr, int file_path_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 1;
    if (g_packs[pack].count >= MAX_ENTRIES) return 1;
    const char *name = (const char *)(intptr_t)name_ptr;
    const char *path = (const char *)(intptr_t)file_path_ptr;
    if (!name || !path) return 1;
    FILE *f = fopen(path, "rb");
    if (!f) return 1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0 || sz > MAX_ENTRY_BYTES) { fclose(f); return 1; }
    unsigned char *buf = (unsigned char *)malloc((size_t)sz ? (size_t)sz : 1);
    if (!buf) { fclose(f); return 1; }
    if (sz && fread(buf, 1, (size_t)sz, f) != (size_t)sz) { free(buf); fclose(f); return 1; }
    fclose(f);
    Entry *e = &g_packs[pack].entries[g_packs[pack].count++];
    memset(e, 0, sizeof(*e));
    strncpy(e->name, name, sizeof(e->name) - 1);
    e->id = rresComputeCRC32((const unsigned char *)name, (int)strlen(name));
    e->data = buf;
    e->size = (unsigned int)sz;
    return 0;
}

int ori_rres_save(int pack, int path_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 1;
    const char *path = (const char *)(intptr_t)path_ptr;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    fwrite("ORPK", 1, 4, f);
    uint32_t ver = 1, count = (uint32_t)g_packs[pack].count;
    fwrite(&ver, 4, 1, f);
    fwrite(&count, 4, 1, f);
    for (int i = 0; i < g_packs[pack].count; ++i) {
        Entry *e = &g_packs[pack].entries[i];
        uint32_t nlen = (uint32_t)strlen(e->name);
        fwrite(&nlen, 4, 1, f);
        fwrite(e->name, 1, nlen, f);
        fwrite(&e->size, 4, 1, f);
        if (e->size) fwrite(e->data, 1, e->size, f);
    }
    fclose(f);
    return 0;
}

/* Validate path only (does not open a pack handle). 0 = ok. */
int ori_rres_validate_path(int path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        set_err("null path");
        return 1;
    }
    FILE *f = fopen(path, "rb");
    if (!f) {
        set_err("cannot open");
        return 1;
    }
    int rc = validate_orpk_file(f, NULL);
    fclose(f);
    return rc;
}

int ori_rres_open(int path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        set_err("null path");
        return -1;
    }
    FILE *f = fopen(path, "rb");
    if (!f) {
        set_err("cannot open");
        return -1;
    }
    /* Validate structure first by scanning, then rewind and load. */
    if (validate_orpk_file(f, NULL) != 0) {
        fclose(f);
        return -1;
    }
    rewind(f);
    char magic[4];
    fread(magic, 1, 4, f);
    uint32_t ver = 0, count = 0;
    fread(&ver, 4, 1, f);
    fread(&count, 4, 1, f);
    int pack = alloc_pack();
    if (pack < 0) {
        fclose(f);
        set_err("too many open packs");
        return -1;
    }
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t nlen = 0, dlen = 0;
        fread(&nlen, 4, 1, f);
        Entry *e = &g_packs[pack].entries[g_packs[pack].count++];
        memset(e, 0, sizeof(*e));
        fread(e->name, 1, nlen, f);
        e->name[nlen] = '\0';
        e->id = rresComputeCRC32((const unsigned char *)e->name, (int)nlen);
        fread(&dlen, 4, 1, f);
        e->data = (unsigned char *)malloc(dlen ? dlen : 1);
        e->size = dlen;
        if (dlen) fread(e->data, 1, dlen, f);
    }
    fclose(f);
    g_packs[pack].valid = 1;
    set_err(NULL);
    return pack;
}

int ori_rres_is_valid(int pack) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 0;
    return g_packs[pack].valid ? 1 : 0;
}

int ori_rres_count(int pack) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 0;
    return g_packs[pack].count;
}

int ori_rres_id_by_name(int pack, int name_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 0;
    const char *name = (const char *)(intptr_t)name_ptr;
    if (!name) return 0;
    unsigned int id = rresComputeCRC32((const unsigned char *)name, (int)strlen(name));
    for (int i = 0; i < g_packs[pack].count; ++i) {
        if (g_packs[pack].entries[i].id == id) return (int)id;
    }
    return 0;
}

int ori_rres_size_by_name(int pack, int name_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return -1;
    const char *name = (const char *)(intptr_t)name_ptr;
    if (!name) return -1;
    unsigned int id = rresComputeCRC32((const unsigned char *)name, (int)strlen(name));
    for (int i = 0; i < g_packs[pack].count; ++i) {
        if (g_packs[pack].entries[i].id == id) return (int)g_packs[pack].entries[i].size;
    }
    return -1;
}

/* Export resource bytes to a filesystem path (for Ori to read via fs). */
int ori_rres_export(int pack, int name_ptr, int out_path_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 1;
    const char *name = (const char *)(intptr_t)name_ptr;
    const char *outp = (const char *)(intptr_t)out_path_ptr;
    if (!name || !outp) return 1;
    unsigned int id = rresComputeCRC32((const unsigned char *)name, (int)strlen(name));
    for (int i = 0; i < g_packs[pack].count; ++i) {
        Entry *e = &g_packs[pack].entries[i];
        if (e->id != id) continue;
        FILE *f = fopen(outp, "wb");
        if (!f) return 1;
        if (e->size) fwrite(e->data, 1, e->size, f);
        fclose(f);
        return 0;
    }
    return 1;
}

/* Alias of export — explicit name for binary reads. */
int ori_rres_read_bytes_to_path(int pack, int name_ptr, int out_path_ptr) {
    return ori_rres_export(pack, name_ptr, out_path_ptr);
}

int ori_rres_has(int pack, int name_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 0;
    const char *name = (const char *)(intptr_t)name_ptr;
    if (!name) return 0;
    unsigned int id = rresComputeCRC32((const unsigned char *)name, (int)strlen(name));
    for (int i = 0; i < g_packs[pack].count; ++i) {
        if (g_packs[pack].entries[i].id == id) return 1;
    }
    return 0;
}

/* Write entry name at index to path (0 = success). */
int ori_rres_name_at_to_path(int pack, int index, int path_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 1;
    if (index < 0 || index >= g_packs[pack].count) return 1;
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    fputs(g_packs[pack].entries[index].name, f);
    fclose(f);
    return 0;
}

/* Write all names, one per line, to path. */
int ori_rres_list_names_to_path(int pack, int path_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 1;
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    for (int i = 0; i < g_packs[pack].count; ++i) {
        fputs(g_packs[pack].entries[i].name, f);
        fputc('\n', f);
    }
    fclose(f);
    return 0;
}

int ori_rres_remove(int pack, int name_ptr) {
    if (pack < 0 || pack >= MAX_PACKS || !g_packs[pack].live) return 1;
    const char *name = (const char *)(intptr_t)name_ptr;
    if (!name) return 1;
    unsigned int id = rresComputeCRC32((const unsigned char *)name, (int)strlen(name));
    for (int i = 0; i < g_packs[pack].count; ++i) {
        if (g_packs[pack].entries[i].id != id) continue;
        free(g_packs[pack].entries[i].data);
        for (int j = i; j < g_packs[pack].count - 1; ++j) {
            g_packs[pack].entries[j] = g_packs[pack].entries[j + 1];
        }
        g_packs[pack].count--;
        memset(&g_packs[pack].entries[g_packs[pack].count], 0, sizeof(Entry));
        return 0;
    }
    return 1;
}

int ori_rres_put_file(int pack, int name_ptr, int file_path_ptr) {
    if (ori_rres_has(pack, name_ptr)) {
        ori_rres_remove(pack, name_ptr);
    }
    return ori_rres_add_file(pack, name_ptr, file_path_ptr);
}

int ori_rres_errmsg_to_path(int path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    fputs(g_last_err, f);
    fclose(f);
    return 0;
}
