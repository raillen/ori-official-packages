/* pack_cli: create ORPK from files.
 * usage: pack_cli out.orpk name1=path1 [name2=path2 ...]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define RRES_IMPLEMENTATION
#include "rres.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s out.orpk name=path [...]\n", argv[0]);
        return 2;
    }
    const char *out = argv[1];
    FILE *f = fopen(out, "wb");
    if (!f) {
        perror(out);
        return 1;
    }
    fwrite("ORPK", 1, 4, f);
    uint32_t ver = 1;
    uint32_t count = (uint32_t)(argc - 2);
    fwrite(&ver, 4, 1, f);
    fwrite(&count, 4, 1, f);
    for (int i = 2; i < argc; ++i) {
        char *eq = strchr(argv[i], '=');
        if (!eq) {
            fprintf(stderr, "bad arg (want name=path): %s\n", argv[i]);
            fclose(f);
            return 1;
        }
        *eq = '\0';
        const char *name = argv[i];
        const char *path = eq + 1;
        FILE *in = fopen(path, "rb");
        if (!in) {
            perror(path);
            fclose(f);
            return 1;
        }
        fseek(in, 0, SEEK_END);
        long sz = ftell(in);
        fseek(in, 0, SEEK_SET);
        unsigned char *buf = (unsigned char *)malloc(sz > 0 ? (size_t)sz : 1);
        if (sz > 0 && fread(buf, 1, (size_t)sz, in) != (size_t)sz) {
            fprintf(stderr, "read fail: %s\n", path);
            free(buf);
            fclose(in);
            fclose(f);
            return 1;
        }
        fclose(in);
        uint32_t nlen = (uint32_t)strlen(name);
        uint32_t dlen = (uint32_t)(sz > 0 ? sz : 0);
        fwrite(&nlen, 4, 1, f);
        fwrite(name, 1, nlen, f);
        fwrite(&dlen, 4, 1, f);
        if (dlen) fwrite(buf, 1, dlen, f);
        free(buf);
        fprintf(stderr, "packed %s (%u bytes) id=0x%08x\n", name, dlen,
                rresComputeCRC32((const unsigned char *)name, (int)nlen));
    }
    fclose(f);
    fprintf(stderr, "wrote %s\n", out);
    return 0;
}
