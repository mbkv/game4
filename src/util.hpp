#pragma once

#include <malloc.h>
#include <assert.h>
#include <stdint.h>

#define force_inline __attribute__((always_inline)) static inline

static_assert(sizeof(size_t) >= sizeof(void *), "This file assumes size_t can hold a void*");

force_inline size_t roundup_by_power2(size_t value, uint8_t exponent) {
	size_t power2 = 1 << exponent;
	// hackers delight chapter 3 p1
	value +=  power2 - 1;

	return value & -power2;
}

typedef void*(*allocator_t)(size_t);
typedef void*(*allocator_free_t)(void *);


// temp_alloc_buffer is 32 byte aligned all the time
/* uint8_t *_temp_alloc_buffer = (uint8_t *)roundup_by_power2((size_t)malloc(1024 * 1024 * 256), 4); */
uint8_t *_temp_alloc_buffer = (uint8_t *)malloc(1024 * 1024 * 256);
uint8_t *_temp_alloc_start = _temp_alloc_buffer;

static void* 
temp_alloc(size_t bytes_requested)
{
	assert((size_t)_temp_alloc_buffer % 16 == 0);
	void* return_value = _temp_alloc_buffer;
	_temp_alloc_buffer =  _temp_alloc_buffer + roundup_by_power2(bytes_requested, 4);
	assert((size_t)_temp_alloc_buffer % 16 == 0);
	return return_value;
}

static void
temp_alloc_free(void *)
{
}

static void
temp_alloc_freeall()
{
	_temp_alloc_buffer = _temp_alloc_start;
}

static void
temp_alloc_debug()
{
	printf("temp_alloc allocated size: %ld\n", _temp_alloc_buffer - _temp_alloc_start);
}







// thank you gingerBill
// https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/

template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})
