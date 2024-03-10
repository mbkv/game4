#pragma once

#include <malloc.h>
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

#define force_inline __attribute__((always_inline)) static inline

static_assert(sizeof(size_t) >= sizeof(void *), "This file assumes size_t can hold a void*");
static_assert(sizeof(size_t) == sizeof(u64), "This file assumes size_t is u64");

template <typename T>
force_inline T max(T a, T b)
{
	return a > b ? a : b;
}

force_inline size_t roundup_by_power2(u64 value, u8 exponent) {
	// hackers delight chapter 3 p1
	u64 power2 = 1 << exponent;
	value +=  power2 - 1;
	return value & -power2;
}

force_inline u8 * roundup_by_power2(u8* value, u8 exponent)
{
	return (u8*)roundup_by_power2((u64) value, exponent);
}

struct allocator_t
{
	void *(*alloc)(u64);
	void (*free)(void *);
};

#define MAX_TEMP_ALLOC_SIZE (1024 * 1024 * 256)

u8 *_temp_alloc_buffer = (u8 *)malloc(MAX_TEMP_ALLOC_SIZE);
u8 *_temp_alloc_start = _temp_alloc_buffer;

static void* 
temp_alloc(u64 bytes_requested)
{
  // we reserve an additional 4 bytes for bytes_requested which is needed for
  // realloc
  _temp_alloc_buffer += 4;
  // we serve 16 bytes alligned all the time
  _temp_alloc_buffer = roundup_by_power2(_temp_alloc_buffer, 4);
  assert((u64)_temp_alloc_buffer % 16 == 0);
  void *return_value = _temp_alloc_buffer;
  u64 *size_value = ((u64 *)_temp_alloc_buffer) - 1;
  *size_value = bytes_requested;
  _temp_alloc_buffer += bytes_requested;
  assert(_temp_alloc_start + MAX_TEMP_ALLOC_SIZE > _temp_alloc_buffer);
  return return_value;
}

static void*
temp_calloc(u64 nmemb, u64 size)
{
	u64 total_size = nmemb * size;
	void* ptr = temp_alloc(total_size);

	memset(ptr, 0, total_size);

	return ptr;
}

static void
temp_alloc_free(void *)
{
	 //do nothing since we need we can free it during a freeall
}

static void
temp_alloc_freeall()
{
	_temp_alloc_buffer = _temp_alloc_start;
}

static void*
temp_alloc_realloc(void *ptr, u64 size)
{
	if (size == 0) {
		temp_alloc_free(ptr);
		return nullptr;
	} else if (ptr == nullptr) {
		return temp_alloc(size);
	}
	u64 *old_size = ((u64 *)ptr) - 1;
	if (*old_size >= size) {
		return ptr;
	}
	// gotta allocate a new array
	void * new_ptr = temp_alloc(size);
	void *new_ptr_end = mempcpy(new_ptr, ptr, *old_size);
	memset(new_ptr_end, 0, size - *old_size);
	return new_ptr;
}

static allocator_t temp_allocator { temp_alloc, temp_alloc_free };

static void
temp_alloc_debug()
{
	printf("temp_alloc allocated size: %ld\n", _temp_alloc_buffer - _temp_alloc_start);
}




// inspired by gingerBill's version
// https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/

#define UNIQUE_VARIABLE_NAME_1(x, y) x##y
#define UNIQUE_VARIABLE_NAME_2(x, y) UNIQUE_VARIABLE_NAME_1(x, y)
#define UNIQUE_VARIABLE_NAME(x)    UNIQUE_VARIABLE_NAME_2(x, __COUNTER__)


template <typename F>
struct defer_t {
	F f;
	defer_t(F f) : f(f) {}
	~defer_t() { f(); }
};

#define defer(code)   defer_t UNIQUE_VARIABLE_NAME(_defer_) {[&](){code;}}
