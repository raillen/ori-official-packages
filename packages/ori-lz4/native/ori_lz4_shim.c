/* LZ4 block + stream/frame bindings for Ori — int64_t ABI for pointers and lengths. */
#if defined(ORI_LZ4_SYSTEM)
#include <lz4.h>
#else
#include "../vendor/lz4.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BUF_CAP (1 << 20)
#define STR_CAP 8192
#define DEFAULT_CHUNK 4096

/* Frame magic: Ori LZ4 stream format v1 (length-prefixed LZ4 blocks + stream state). */
static const unsigned char MAGIC_OLZ1[4] = { 'O', 'L', 'Z', '1' };

static unsigned char g_out[BUF_CAP];
static unsigned char g_tmp[BUF_CAP];
static unsigned char g_plain[BUF_CAP];
static int g_out_len = 0;
static int g_src_len = 0; /* last plain input length */
static char g_str_in[STR_CAP];

static void write_u32_le(unsigned char *p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
    p[2] = (unsigned char)((v >> 16) & 0xffu);
    p[3] = (unsigned char)((v >> 24) & 0xffu);
}

static int read_u32_le(const unsigned char *p, uint32_t *out) {
    if (!p || !out) {
        return 0;
    }
    *out = (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
    return 1;
}

/* ---- bound / size helpers ---- */

int64_t ori_lz4_compress_bound(int64_t src_nbytes) {
    if (src_nbytes <= 0 || src_nbytes > (int64_t)LZ4_MAX_INPUT_SIZE) {
        return -1;
    }
    return (int64_t)LZ4_compressBound((int)src_nbytes);
}

/* Worst-case OLZ1 frame size for src_nbytes with the given chunk size. */
int64_t ori_lz4_stream_bound(int64_t src_nbytes, int64_t chunk_size) {
    if (src_nbytes < 0) {
        return -1;
    }
    if (src_nbytes == 0) {
        /* magic + plain + chunk + terminator */
        return 4 + 4 + 4 + 4;
    }
    int chunk = (int)chunk_size;
    if (chunk <= 0) {
        chunk = DEFAULT_CHUNK;
    }
    if (chunk > BUF_CAP / 4) {
        chunk = BUF_CAP / 4;
    }
    int64_t nchunks = (src_nbytes + chunk - 1) / chunk;
    int64_t bound = 4 + 4 + 4; /* magic + plain_total + chunk_size */
    int64_t i;
    for (i = 0; i < nchunks; i++) {
        int64_t remain = src_nbytes - i * chunk;
        int csz = remain < chunk ? (int)remain : chunk;
        int cbound = LZ4_compressBound(csz);
        if (cbound <= 0) {
            return -1;
        }
        bound += 4 + cbound; /* u32 clen + payload */
    }
    bound += 4; /* terminator clen=0 */
    return bound;
}

/* ---- block compress / decompress ---- */

int64_t ori_lz4_compress(int64_t src_ptr, int64_t src_nbytes) {
    g_out_len = 0;
    g_src_len = 0;
    if (!src_ptr || src_nbytes <= 0 || src_nbytes > BUF_CAP / 2) {
        return -1;
    }
    int bound = LZ4_compressBound((int)src_nbytes);
    if (bound <= 0 || bound > BUF_CAP) {
        return -1;
    }
    int clen = LZ4_compress_default(
        (const char *)(intptr_t)src_ptr,
        (char *)g_out,
        (int)src_nbytes,
        bound
    );
    if (clen <= 0) {
        return -1;
    }
    g_out_len = clen;
    g_src_len = (int)src_nbytes;
    return (int64_t)g_out_len;
}

/* Decompress compressed blob at src_ptr into g_out. max_out is dst capacity (0 = BUF_CAP). */
int64_t ori_lz4_decompress(int64_t src_ptr, int64_t src_nbytes, int64_t max_out) {
    if (!src_ptr || src_nbytes <= 0) {
        return -1;
    }
    if (src_nbytes > BUF_CAP) {
        return -1;
    }
    /* Copy source first — may alias g_out after compress. */
    memcpy(g_tmp, (const void *)(intptr_t)src_ptr, (size_t)src_nbytes);
    int dst_cap = (int)(max_out > 0 && max_out < BUF_CAP ? max_out : BUF_CAP);
    int ulen = LZ4_decompress_safe(
        (const char *)g_tmp,
        (char *)g_out,
        (int)src_nbytes,
        dst_cap
    );
    if (ulen < 0) {
        g_out_len = 0;
        return -1;
    }
    g_out_len = ulen;
    return (int64_t)g_out_len;
}

/* ---- stream / frame (OLZ1) ---- */

/*
 * OLZ1 layout:
 *   magic[4] = 'O','L','Z','1'
 *   u32le plain_total
 *   u32le chunk_size
 *   repeated:
 *     u32le compressed_size
 *     compressed_size bytes  (LZ4_compress_fast_continue)
 *   u32le 0  (terminator)
 */

int64_t ori_lz4_stream_compress(int64_t src_ptr, int64_t src_nbytes, int64_t chunk_size) {
    g_out_len = 0;
    g_src_len = 0;
    if (!src_ptr || src_nbytes < 0 || src_nbytes > BUF_CAP / 2) {
        return -1;
    }

    int chunk = (int)chunk_size;
    if (chunk <= 0) {
        chunk = DEFAULT_CHUNK;
    }
    if (chunk > BUF_CAP / 4) {
        chunk = BUF_CAP / 4;
    }

    int64_t need = ori_lz4_stream_bound(src_nbytes, chunk);
    if (need < 0 || need > BUF_CAP) {
        return -1;
    }

    LZ4_stream_t *stream = LZ4_createStream();
    if (!stream) {
        return -1;
    }

    const unsigned char *src = (const unsigned char *)(intptr_t)src_ptr;
    unsigned char *dst = g_out;
    size_t off = 0;

    memcpy(dst + off, MAGIC_OLZ1, 4);
    off += 4;
    write_u32_le(dst + off, (uint32_t)src_nbytes);
    off += 4;
    write_u32_le(dst + off, (uint32_t)chunk);
    off += 4;

    int64_t remaining = src_nbytes;
    int64_t src_off = 0;
    while (remaining > 0) {
        int csz = remaining < chunk ? (int)remaining : chunk;
        int cbound = LZ4_compressBound(csz);
        if (cbound <= 0 || (int64_t)off + 4 + cbound + 4 > BUF_CAP) {
            LZ4_freeStream(stream);
            return -1;
        }
        /* compress into place after the length field */
        int clen = LZ4_compress_fast_continue(
            stream,
            (const char *)(src + src_off),
            (char *)(dst + off + 4),
            csz,
            cbound,
            1
        );
        if (clen <= 0) {
            LZ4_freeStream(stream);
            return -1;
        }
        write_u32_le(dst + off, (uint32_t)clen);
        off += 4 + (size_t)clen;
        src_off += csz;
        remaining -= csz;
    }

    if ((int64_t)off + 4 > BUF_CAP) {
        LZ4_freeStream(stream);
        return -1;
    }
    write_u32_le(dst + off, 0);
    off += 4;

    LZ4_freeStream(stream);
    g_out_len = (int)off;
    g_src_len = (int)src_nbytes;
    return (int64_t)g_out_len;
}

int64_t ori_lz4_stream_decompress(int64_t src_ptr, int64_t src_nbytes, int64_t max_out) {
    g_out_len = 0;
    if (!src_ptr || src_nbytes < 16) {
        return -1;
    }
    if (src_nbytes > BUF_CAP) {
        return -1;
    }

    memcpy(g_tmp, (const void *)(intptr_t)src_ptr, (size_t)src_nbytes);
    const unsigned char *src = g_tmp;
    size_t off = 0;

    if (memcmp(src + off, MAGIC_OLZ1, 4) != 0) {
        return -1;
    }
    off += 4;

    uint32_t plain_total = 0;
    uint32_t chunk_size = 0;
    if (!read_u32_le(src + off, &plain_total)) {
        return -1;
    }
    off += 4;
    if (!read_u32_le(src + off, &chunk_size)) {
        return -1;
    }
    off += 4;

    int dst_cap = (int)(max_out > 0 && max_out < BUF_CAP ? max_out : BUF_CAP);
    if ((int)plain_total > dst_cap) {
        return -1;
    }

    LZ4_streamDecode_t *dec = LZ4_createStreamDecode();
    if (!dec) {
        return -1;
    }
    if (!LZ4_setStreamDecode(dec, NULL, 0)) {
        LZ4_freeStreamDecode(dec);
        return -1;
    }

    int written = 0;
    while (off + 4 <= (size_t)src_nbytes) {
        uint32_t clen = 0;
        if (!read_u32_le(src + off, &clen)) {
            LZ4_freeStreamDecode(dec);
            return -1;
        }
        off += 4;
        if (clen == 0) {
            break;
        }
        if (off + clen > (size_t)src_nbytes) {
            LZ4_freeStreamDecode(dec);
            return -1;
        }
        int room = dst_cap - written;
        if (room <= 0) {
            LZ4_freeStreamDecode(dec);
            return -1;
        }
        int ulen = LZ4_decompress_safe_continue(
            dec,
            (const char *)(src + off),
            (char *)(g_out + written),
            (int)clen,
            room
        );
        if (ulen < 0) {
            LZ4_freeStreamDecode(dec);
            g_out_len = 0;
            return -1;
        }
        written += ulen;
        off += clen;
    }

    LZ4_freeStreamDecode(dec);

    if ((uint32_t)written != plain_total) {
        g_out_len = 0;
        return -1;
    }
    g_out_len = written;
    return (int64_t)g_out_len;
}

/* ---- text helpers (null-terminated UTF-8) ---- */

int64_t ori_lz4_compress_cstr(int64_t text_ptr) {
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
    return ori_lz4_compress((int64_t)(intptr_t)g_str_in, (int64_t)n);
}

int64_t ori_lz4_stream_compress_cstr(int64_t text_ptr, int64_t chunk_size) {
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
    return ori_lz4_stream_compress((int64_t)(intptr_t)g_str_in, (int64_t)n, chunk_size);
}

int64_t ori_lz4_decompress_last_as_cstr(void) {
    if (g_out_len <= 0 || g_out_len >= (int)sizeof(g_str_in)) {
        return 1;
    }
    memcpy(g_str_in, g_out, (size_t)g_out_len);
    g_str_in[g_out_len] = '\0';
    return 0;
}

int64_t ori_lz4_cstr_ptr(void) { return (int64_t)(intptr_t)g_str_in; }

/* ---- accessors ---- */

int64_t ori_lz4_out_len(void) { return (int64_t)g_out_len; }
int64_t ori_lz4_out_ptr(void) { return (int64_t)(intptr_t)g_out; }
int64_t ori_lz4_src_len(void) { return (int64_t)g_src_len; }

int64_t ori_lz4_out_byte(int64_t i) {
    if (i < 0 || i >= g_out_len) {
        return 0;
    }
    return (int64_t)g_out[i];
}

int64_t ori_lz4_out_sum(void) {
    int64_t s = 0;
    int i;
    for (i = 0; i < g_out_len; i++) {
        s += (int64_t)g_out[i];
    }
    return s;
}

/* ---- larger fixture: pattern buffer round-trip (block + stream) ---- */

static void fill_pattern(unsigned char *buf, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        /* mix of runs + variation so compress is non-trivial but valid */
        buf[i] = (unsigned char)((i * 17 + (i / 64)) & 0xff);
    }
}

/* Compress then decompress a pattern of nbytes; returns plain length on match, else <0. */
int64_t ori_lz4_pattern_roundtrip(int64_t nbytes) {
    if (nbytes <= 0 || nbytes > BUF_CAP / 2) {
        return -1;
    }
    fill_pattern(g_plain, (int)nbytes);
    int64_t clen = ori_lz4_compress((int64_t)(intptr_t)g_plain, nbytes);
    if (clen <= 0) {
        return -1;
    }
    /* Snapshot compressed bytes — decompress overwrites g_out. */
    if (clen > BUF_CAP) {
        return -1;
    }
    memcpy(g_tmp, g_out, (size_t)clen);
    int64_t ulen = ori_lz4_decompress((int64_t)(intptr_t)g_tmp, clen, nbytes);
    if (ulen != nbytes) {
        return -1;
    }
    if (memcmp(g_out, g_plain, (size_t)nbytes) != 0) {
        return -1;
    }
    return ulen;
}

int64_t ori_lz4_pattern_stream_roundtrip(int64_t nbytes, int64_t chunk_size) {
    if (nbytes <= 0 || nbytes > BUF_CAP / 2) {
        return -1;
    }
    fill_pattern(g_plain, (int)nbytes);
    int64_t flen = ori_lz4_stream_compress((int64_t)(intptr_t)g_plain, nbytes, chunk_size);
    if (flen <= 0) {
        return -1;
    }
    if (flen > BUF_CAP) {
        return -1;
    }
    memcpy(g_tmp, g_out, (size_t)flen);
    int64_t ulen = ori_lz4_stream_decompress((int64_t)(intptr_t)g_tmp, flen, nbytes);
    if (ulen != nbytes) {
        return -1;
    }
    if (memcmp(g_out, g_plain, (size_t)nbytes) != 0) {
        return -1;
    }
    return ulen;
}
