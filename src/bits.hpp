#pragma once

#include "./util.hpp"
#include <limits.h>

#define BIT_SIZE(T) sizeof(T) * CHAR_BIT

force_inline u64 fill_least_bits(u8 bits) {
    u64 value = -1;
    value = value >> (BIT_SIZE(u64) - bits);
    return value;
}

force_inline u8 most_significant_bit(u64 value) {
    if (value == 0) {
        return 0;
    }

    u8 total_bits = BIT_SIZE(decltype(value));
    u8 leading_bits = __builtin_clzll(value);
    return total_bits - leading_bits;
}

force_inline u8 least_significant_bit(u64 value) {
    if (value == 0) {
        return 64;
    }
    u8 trailing_bits = __builtin_ctzll(value);

    return trailing_bits;
}

