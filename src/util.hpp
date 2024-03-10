#pragma once

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#define force_inline __attribute__((always_inline)) static inline

#define ARRAY_LEN(x) ((sizeof(x)) / (sizeof(x[0])))

template <typename T> force_inline T max(T a, T b) { return a > b ? a : b; }

force_inline size_t roundup_by_power2(u64 value, u8 exponent) {
    // hackers delight chapter 3 p1
    u64 power2 = 1 << exponent;
    value += power2 - 1;
    return value & -power2;
}

force_inline u8 *roundup_by_power2(u8 *value, u8 exponent) {
    return (u8 *)roundup_by_power2((u64)value, exponent);
}

// inspired by gingerBill's version
// https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/

#define UNIQUE_VARIABLE_NAME_1(x, y) x##y
#define UNIQUE_VARIABLE_NAME_2(x, y) UNIQUE_VARIABLE_NAME_1(x, y)
#define UNIQUE_VARIABLE_NAME(x) UNIQUE_VARIABLE_NAME_2(x, __COUNTER__)

template <typename F> struct defer_t {
    F f;
    defer_t(F f) : f(f) {}
    ~defer_t() { f(); }
};

#define defer(code)                                                                    \
    defer_t UNIQUE_VARIABLE_NAME(_defer_) {                                            \
        [&]() { code; }                                                                \
    }
