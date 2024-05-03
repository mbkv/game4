#pragma once

#include "src/util.hpp"
#include "src/os.hpp"
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <nmmintrin.h>

struct _profile {
    const char *function;
    size_t line;
    size_t runs;
    high_frequency_timer_t start;

    explicit _profile(const char *_function, size_t _line, size_t _runs)
        : function(_function), line(_line), runs(_runs) {
        start = get_high_frequency_time();
    }
    ~_profile() {
        high_frequency_timer_t total = get_high_frequency_time() - start;
        printf("%s:%lu: %f\t%f\n", function, line, total, f32(total) / runs);
    }
};

#define PROFILE1(x) (x)
#define PROFILE2(x) PROFILE1(x)
#define profile(runs)                                                                  \
    _profile UNIQUE_VARIABLE_NAME(_profile_)(__PRETTY_FUNCTION__, __LINE__, runs)
