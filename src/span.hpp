#pragma once

#include "src/util.hpp"

template <typename T>
struct span {
    T *ptr;
    size_t len;

    T &operator[](size_t i) {
#ifdef SLOW
        assert(i < len);
#endif

        return ptr[i];
    }
};

