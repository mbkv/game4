#pragma once

#ifdef __linux__

#include "src/util.hpp"

#if 0
#include "src/alloc_ctx.hpp"
#include "src/string.hpp"
#include <stdio.h>

static string read_entire_file(string_view filename) {
    FILE *f = fopen(filename.data(), "rb");
    if (f == nullptr) {
        fprintf(stderr, "Could not open '%s'", filename.data());
        return string_make();
    }
    defer(fclose(f));

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    auto [str, ptr] = string_make(fsize);

    fread(ptr, fsize, 1, f);

    ptr[fsize] = 0;

    return str;
}

static void read_entire_file_free(string *file) {
    string_destroy(file);
}

#endif

#define resource_get_path(file) ("res/" file)

typedef u64 high_frequency_timer_t;

static high_frequency_timer_t get_high_frequency_time() {
    return __builtin_ia32_rdtsc();
}

#endif
