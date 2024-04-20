#pragma once

#include "./util.hpp"
#include <stdio.h>
#include <x86intrin.h>

struct _profile {
    const char *function;
    size_t line;
    size_t runs;
    u64 start;
    explicit _profile(const char *_function, size_t _line, size_t _runs)
        : function(_function), line(_line), runs(_runs) {
        start = __rdtsc();
    }
    ~_profile() {
        u64 total = __rdtsc() - start;
        fprintf(stderr, "%s:%lu: %lu\t%f\n", function, line, total, f32(total) / runs);
    }
};

#define PROFILE1(x) (x)
#define PROFILE2(x) PROFILE1(x)
#define profile(runs)                                                                  \
    _profile UNIQUE_VARIABLE_NAME(_profile_)(__PRETTY_FUNCTION__, __LINE__, runs)
