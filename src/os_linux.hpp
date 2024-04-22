#pragma once

#ifdef __linux__

#include "src/alloc_ctx.hpp"
#include "src/string.hpp"
#include <stdio.h>

static async read_entire_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f == nullptr) {
        fprintf(stderr, "Could not open '%s'", filename);
        return {nullptr, 0};
    }
    defer(fclose(f));

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    char *s = (char *)global_ctx->alloc(fsize + 1);
    fread(s, fsize, 1, f);

    s[fsize] = 0;

    return {s, (u64)fsize};
}

static void read_entire_file_free(str *file) {
    assert(file);
    assert(file->s);

    global_ctx->free(file->s);
    file->s = nullptr;
    file->len = 0;
}

#define resource_get_path(file) ("res/" file)

#endif
