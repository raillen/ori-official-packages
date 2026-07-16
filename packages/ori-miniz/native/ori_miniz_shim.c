/* miniz deflate/CRC/zip bindings for Ori — int64_t ABI (pointers + lengths). */
#include "../vendor/miniz.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BUF_CAP (1 << 20)
static unsigned char g_out[BUF_CAP];
static unsigned char g_tmp[BUF_CAP];
static int g_out_len = 0;
static int g_src_len = 0;
static char g_str_in[4096];
static char g_name[512];

/* ---- deflate buffer ---- */

int64_t ori_mz_compress(int64_t src_ptr, int64_t src_len, int64_t level) {
    g_out_len = 0;
    g_src_len = 0;
    if (!src_ptr || src_len <= 0 || src_len > BUF_CAP / 2) {
        return -1;
    }
    mz_ulong out_len = BUF_CAP;
    int rc = mz_compress2(
        g_out,
        &out_len,
        (const unsigned char *)(intptr_t)src_ptr,
        (mz_ulong)src_len,
        (int)level
    );
    if (rc != MZ_OK) {
        return -1;
    }
    g_out_len = (int)out_len;
    g_src_len = (int)src_len;
    return (int64_t)g_out_len;
}

int64_t ori_mz_uncompress(int64_t src_ptr, int64_t src_len, int64_t max_out) {
    if (!src_ptr || src_len <= 0) {
        return -1;
    }
    /* Copy source first — may alias g_out after compress. */
    if (src_len > BUF_CAP) {
        return -1;
    }
    memcpy(g_tmp, (const void *)(intptr_t)src_ptr, (size_t)src_len);
    mz_ulong out_len = (mz_ulong)(max_out > 0 && max_out < BUF_CAP ? max_out : BUF_CAP);
    int rc = mz_uncompress(g_out, &out_len, g_tmp, (mz_ulong)src_len);
    if (rc != MZ_OK) {
        g_out_len = 0;
        return -1;
    }
    g_out_len = (int)out_len;
    return (int64_t)g_out_len;
}

int64_t ori_mz_out_len(void) { return (int64_t)g_out_len; }
int64_t ori_mz_out_ptr(void) { return (int64_t)(intptr_t)g_out; }
int64_t ori_mz_src_len(void) { return (int64_t)g_src_len; }

int64_t ori_mz_out_byte(int64_t i) {
    if (i < 0 || i >= g_out_len) {
        return 0;
    }
    return (int64_t)g_out[i];
}

/* ---- text helpers (null-terminated UTF-8) ---- */

int64_t ori_mz_compress_cstr(int64_t text_ptr, int64_t level) {
    const char *s = (const char *)(intptr_t)text_ptr;
    if (!s) {
        return -1;
    }
    size_t n = strlen(s);
    if (n >= sizeof(g_str_in)) {
        n = sizeof(g_str_in) - 1;
    }
    memcpy(g_str_in, s, n);
    g_str_in[n] = '\0';
    return ori_mz_compress((int64_t)(intptr_t)g_str_in, (int64_t)n, level);
}

int64_t ori_mz_uncompress_last_as_cstr(void) {
    if (g_out_len <= 0 || g_out_len >= (int)sizeof(g_str_in)) {
        return 1;
    }
    memcpy(g_str_in, g_out, (size_t)g_out_len);
    g_str_in[g_out_len] = '\0';
    return 0;
}

int64_t ori_mz_cstr_ptr(void) { return (int64_t)(intptr_t)g_str_in; }

/* ---- CRC32 ---- */

int64_t ori_mz_crc32(int64_t ptr, int64_t len) {
    if (!ptr || len < 0) {
        return 0;
    }
    return (int64_t)mz_crc32(MZ_CRC32_INIT, (const unsigned char *)(intptr_t)ptr, (size_t)len);
}

int64_t ori_mz_crc32_cstr(int64_t text_ptr) {
    const char *s = (const char *)(intptr_t)text_ptr;
    if (!s) {
        return 0;
    }
    return ori_mz_crc32((int64_t)(intptr_t)s, (int64_t)strlen(s));
}

/* ---- ZIP: create one entry / extract one entry (in-memory) ---- */

static int copy_cstr(char *dst, size_t dst_cap, int64_t ptr) {
    const char *s = (const char *)(intptr_t)ptr;
    if (!s || dst_cap == 0) {
        return 0;
    }
    size_t n = strlen(s);
    if (n >= dst_cap) {
        n = dst_cap - 1;
    }
    memcpy(dst, s, n);
    dst[n] = '\0';
    return 1;
}

/* Build a single-entry ZIP into g_out. Returns archive byte length (<0 fail). */
int64_t ori_mz_zip_create_one(
    int64_t name_ptr,
    int64_t data_ptr,
    int64_t data_len,
    int64_t level
) {
    g_out_len = 0;
    if (!name_ptr || !data_ptr || data_len < 0 || data_len > BUF_CAP / 2) {
        return -1;
    }
    if (!copy_cstr(g_name, sizeof(g_name), name_ptr)) {
        return -1;
    }

    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_writer_init_heap(&zip, 0, 128 * 1024)) {
        return -1;
    }

    mz_uint level_flags = (mz_uint)(level < 0 ? MZ_DEFAULT_COMPRESSION : (int)level);
    if (!mz_zip_writer_add_mem(
            &zip,
            g_name,
            (const void *)(intptr_t)data_ptr,
            (size_t)data_len,
            level_flags
        )) {
        mz_zip_writer_end(&zip);
        return -1;
    }

    void *buf = NULL;
    size_t size = 0;
    if (!mz_zip_writer_finalize_heap_archive(&zip, &buf, &size)) {
        mz_zip_writer_end(&zip);
        return -1;
    }
    mz_zip_writer_end(&zip);

    if (!buf || size == 0 || size > (size_t)BUF_CAP) {
        if (buf) {
            mz_free(buf);
        }
        return -1;
    }
    memcpy(g_out, buf, size);
    g_out_len = (int)size;
    mz_free(buf);
    return (int64_t)g_out_len;
}

int64_t ori_mz_zip_create_one_cstr(int64_t name_ptr, int64_t text_ptr, int64_t level) {
    const char *s = (const char *)(intptr_t)text_ptr;
    if (!s) {
        return -1;
    }
    size_t n = strlen(s);
    if (n >= sizeof(g_str_in)) {
        n = sizeof(g_str_in) - 1;
    }
    memcpy(g_str_in, s, n);
    g_str_in[n] = '\0';
    return ori_mz_zip_create_one(name_ptr, (int64_t)(intptr_t)g_str_in, (int64_t)n, level);
}

/* Extract named entry from ZIP at zip_ptr into g_out. Returns plain length (<0 fail). */
int64_t ori_mz_zip_extract_one(int64_t zip_ptr, int64_t zip_len, int64_t name_ptr) {
    if (!zip_ptr || zip_len <= 0 || zip_len > BUF_CAP || !name_ptr) {
        return -1;
    }
    if (!copy_cstr(g_name, sizeof(g_name), name_ptr)) {
        return -1;
    }
    /* Copy archive first — may alias g_out after create. */
    memcpy(g_tmp, (const void *)(intptr_t)zip_ptr, (size_t)zip_len);

    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_mem(&zip, g_tmp, (size_t)zip_len, 0)) {
        g_out_len = 0;
        return -1;
    }

    size_t out_size = 0;
    void *plain = mz_zip_reader_extract_file_to_heap(&zip, g_name, &out_size, 0);
    if (!plain) {
        mz_zip_reader_end(&zip);
        g_out_len = 0;
        return -1;
    }
    if (out_size > (size_t)BUF_CAP) {
        mz_free(plain);
        mz_zip_reader_end(&zip);
        g_out_len = 0;
        return -1;
    }
    memcpy(g_out, plain, out_size);
    g_out_len = (int)out_size;
    mz_free(plain);
    mz_zip_reader_end(&zip);
    return (int64_t)g_out_len;
}

/* Extract named entry from the last g_out archive (create → extract round-trip). */
int64_t ori_mz_zip_extract_last(int64_t name_ptr) {
    if (g_out_len <= 0) {
        return -1;
    }
    return ori_mz_zip_extract_one((int64_t)(intptr_t)g_out, (int64_t)g_out_len, name_ptr);
}

/* First byte sum of last out (stable hash for tests without full memcmp). */
int64_t ori_mz_out_sum(void) {
    int64_t s = 0;
    int i;
    for (i = 0; i < g_out_len; i++) {
        s += (int64_t)g_out[i];
    }
    return s;
}
