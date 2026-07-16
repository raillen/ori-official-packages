/* PhysicsFS bindings for Ori — int64_t ABI for pointers and lengths. */
#if defined(ORI_PHYSFS_SYSTEM)
#include <physfs.h>
#else
#include "physfs.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define READ_CAP (1 << 20)

static unsigned char g_buf[READ_CAP];
static int64_t g_len = 0;
static int64_t g_sum = 0;
static int64_t g_enum_count = 0;
static int64_t g_write_len = 0;

static PHYSFS_EnumerateCallbackResult ori_enum_cb(
    void *data,
    const char *origdir,
    const char *fname
) {
    (void)data;
    (void)origdir;
    (void)fname;
    g_enum_count++;
    return PHYSFS_ENUM_OK;
}

/* Returns 0 on success, 1 on failure. */
int64_t ori_physfs_init(void) {
    g_len = 0;
    g_sum = 0;
    g_enum_count = 0;
    g_write_len = 0;
    if (PHYSFS_isInit()) {
        return 0;
    }
    return PHYSFS_init(NULL) ? 0 : 1;
}

int64_t ori_physfs_deinit(void) {
    g_len = 0;
    g_sum = 0;
    g_enum_count = 0;
    g_write_len = 0;
    if (!PHYSFS_isInit()) {
        return 0;
    }
    return PHYSFS_deinit() ? 0 : 1;
}

/* appendToPath = 1 so multiple mounts stack. Empty mountPoint mounts at root. */
int64_t ori_physfs_mount(int64_t path_ptr, int64_t mount_point_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    const char *mp = (const char *)(intptr_t)mount_point_ptr;
    if (!path) {
        return 1;
    }
    if (!mp) {
        mp = "";
    }
    return PHYSFS_mount(path, mp, 1) ? 0 : 1;
}

int64_t ori_physfs_unmount(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        return 1;
    }
    return PHYSFS_unmount(path) ? 0 : 1;
}

/* 1 if exists, 0 if not. */
int64_t ori_physfs_exists(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        return 0;
    }
    return PHYSFS_exists(path) ? 1 : 0;
}

/*
 * Read entire file into internal buffer.
 * Returns length (>=0) on success, -1 on failure.
 */
int64_t ori_physfs_read(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    g_len = 0;
    g_sum = 0;
    if (!path) {
        return -1;
    }
    PHYSFS_File *f = PHYSFS_openRead(path);
    if (!f) {
        return -1;
    }
    PHYSFS_sint64 flen = PHYSFS_fileLength(f);
    if (flen < 0 || flen > READ_CAP) {
        PHYSFS_close(f);
        return -1;
    }
    PHYSFS_sint64 got = PHYSFS_readBytes(f, g_buf, (PHYSFS_uint64)flen);
    PHYSFS_close(f);
    if (got != flen) {
        return -1;
    }
    g_len = (int64_t)got;
    for (int64_t i = 0; i < g_len; ++i) {
        g_sum += g_buf[i];
    }
    return g_len;
}

int64_t ori_physfs_last_len(void) {
    return g_len;
}

int64_t ori_physfs_last_sum(void) {
    return g_sum;
}

int64_t ori_physfs_last_byte(int64_t i) {
    if (i < 0 || i >= g_len) {
        return 0;
    }
    return (int64_t)g_buf[i];
}

int64_t ori_physfs_last_ptr(void) {
    return (int64_t)(intptr_t)g_buf;
}

/* Count entries in a virtual directory (non-recursive). Returns count or -1. */
int64_t ori_physfs_enumerate_count(int64_t dir_ptr) {
    const char *dir = (const char *)(intptr_t)dir_ptr;
    if (!dir) {
        dir = "";
    }
    g_enum_count = 0;
    if (PHYSFS_enumerate(dir, ori_enum_cb, NULL) == 0) {
        return -1;
    }
    return g_enum_count;
}

/*
 * Set real directory for PHYSFS writes (does not auto-mount).
 * Returns 0 on success, 1 on failure.
 */
int64_t ori_physfs_set_write_dir(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        return 1;
    }
    return PHYSFS_setWriteDir(path) ? 0 : 1;
}

/*
 * Create virtual directory tree under the write dir.
 * Returns 0 on success, 1 on failure.
 */
int64_t ori_physfs_mkdir(int64_t path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        return 1;
    }
    return PHYSFS_mkdir(path) ? 0 : 1;
}

/*
 * Write bytes to a virtual path (requires set_write_dir).
 * len < 0 means data_ptr is a C string (NUL-terminated); length via strlen.
 * Returns bytes written (>=0) on success, -1 on failure.
 */
int64_t ori_physfs_write(int64_t path_ptr, int64_t data_ptr, int64_t len) {
    const char *path = (const char *)(intptr_t)path_ptr;
    const char *data = (const char *)(intptr_t)data_ptr;
    g_write_len = 0;
    if (!path || !data) {
        return -1;
    }
    if (len < 0) {
        len = (int64_t)strlen(data);
    }
    if (len == 0) {
        /* Create empty file. */
        PHYSFS_File *f0 = PHYSFS_openWrite(path);
        if (!f0) {
            return -1;
        }
        PHYSFS_close(f0);
        g_write_len = 0;
        return 0;
    }
    PHYSFS_File *f = PHYSFS_openWrite(path);
    if (!f) {
        return -1;
    }
    PHYSFS_sint64 wrote = PHYSFS_writeBytes(f, data, (PHYSFS_uint64)len);
    PHYSFS_close(f);
    if (wrote < 0 || wrote != (PHYSFS_sint64)len) {
        return -1;
    }
    g_write_len = (int64_t)wrote;
    return g_write_len;
}

int64_t ori_physfs_last_write_len(void) {
    return g_write_len;
}
