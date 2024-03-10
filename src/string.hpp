#pragma once

#include "./util.hpp"
#include <stdio.h>

constexpr u32 fnv1a_32(const char *s) {
    u32 prime = 16777619;
    u32 offset = 2166136261;

    u32 hash = offset;
    while (*s) {
        hash ^= *s;
        hash *= prime;
    }

    return hash;
}

constexpr u64 fnv1a_64(const char *s) {
    u64 prime = 1099511628211ull;
    u64 offset = 14695981039346656037ull;

    u64 hash = offset;
    while (*s) {
        hash ^= *s;
        hash *= prime;
    }

    return hash;
}

struct readonly_str {
    const char *s;
    u64 len;
};

struct str {
    char *s;
    u64 len;
};
