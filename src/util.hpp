#pragma once

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <type_traits>

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

#define PI 3.141592f
#define TAU (PI * 2)

#define force_inline __attribute__((always_inline)) static inline

#define ARRAY_LEN(x) ((sizeof(x)) / (sizeof(x[0])))

template <typename T> force_inline T max(T a, T b) { return a > b ? a : b; }

force_inline bool is_power2(u64 power2) {
    bool is_power2 = (power2 & (power2 - 1)) == 0;

    return is_power2;
}

template <typename T> force_inline T roundup_by_power2(T value, u64 power2) {
    static_assert(std::is_integral<T>());
#if SLOW
    assert(is_power2(power2));
#endif
    // hackers delight chapter 3 p1
    value += power2 - 1;
    return value & -power2;
}

template <typename T> force_inline T divide_roundup(T value, T divisor) {
    static_assert(std::is_integral<T>());
    T rounded_up = (value + divisor - 1) / divisor;

    return rounded_up;
}

template <typename T> force_inline T clamp(T value, T a, T b) {
    if (value < a) {
        return a;
    }
    if (value > b) {
        return b;
    }
    return value;
}

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

force_inline f32 to_radians(f32 degrees) {
    f32 radians = degrees / 180.0f * PI;
    return radians;
}
