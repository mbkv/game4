#pragma once

#include "src/util.hpp"
#include <time.h>

struct rand64_state {
    u64 _state;
};

force_inline rand64_state rand64_seed(u64 seed) {
    if (seed == 0) {
        time_t timer;
        time(&timer);
        seed = (u64) timer;
    }

    return {seed};
}

static u64 rand64(rand64_state *state)
{
    u64 x = state->_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    state->_state = x;
    return x;
}

