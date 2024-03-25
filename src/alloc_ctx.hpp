#pragma once

#include "./util.hpp"
#include <malloc.h>

static_assert(sizeof(size_t) >= sizeof(void *),
              "This file assumes size_t can hold a void*");
static_assert(sizeof(size_t) == sizeof(u64), "This file assumes size_t is u64");

struct allocator_t {
    void *(*alloc)(u64);
    void *(*calloc)(u64, u64);
    void *(*realloc)(void *, u64);
    void (*free)(void *);
};

#define MAX_TEMP_ALLOC_SIZE (1024 * 1024 * 256)

u8 *_temp_alloc_buffer = (u8 *)malloc(MAX_TEMP_ALLOC_SIZE);
u8 *_temp_alloc_start = _temp_alloc_buffer;

static void *temp_alloc_alloc(u64 bytes_requested) {
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

static void *temp_alloc_calloc(u64 nmemb, u64 size) {
    u64 total_size = nmemb * size;
    void *ptr = temp_alloc_alloc(total_size);

    memset(ptr, 0, total_size);

    return ptr;
}

static void temp_alloc_free(void *) {
    // do nothing since we need we can free it during a freeall
}

static void temp_alloc_freeall() { _temp_alloc_buffer = _temp_alloc_start; }

static void *temp_alloc_realloc(void *ptr, u64 size) {
    if (size == 0) {
        temp_alloc_free(ptr);
        return nullptr;
    } else if (ptr == nullptr) {
        return temp_alloc_alloc(size);
    }
    u64 *old_size = ((u64 *)ptr) - 1;
    if (*old_size >= size) {
        return ptr;
    }
    // gotta allocate a new array
    void *new_ptr = temp_alloc_alloc(size);
    void *new_ptr_end = mempcpy(new_ptr, ptr, *old_size);
    memset(new_ptr_end, 0, size - *old_size);
    return new_ptr;
}

static void temp_alloc_debug() {
    printf("temp_alloc allocated size: %ld\n", _temp_alloc_buffer - _temp_alloc_start);
}

static allocator_t temp_allocator{temp_alloc_alloc, temp_alloc_calloc,
                                  temp_alloc_realloc, temp_alloc_free};

struct global_context : allocator_t {
    allocator_t temp;
};

const global_context _alloc_ctx{{malloc, calloc, realloc, free}, temp_allocator};
const global_context _temp_ctx{{temp_allocator}, temp_allocator};

bool _global_ctx_is_temporary = false;
global_context const *global_ctx = &_alloc_ctx;

static void _global_ctx_noop() {}
static void global_ctx_set_default() {
    _global_ctx_is_temporary = false;
    global_ctx = &_alloc_ctx;
}

static defer_t<void (*)()> global_ctx_set_temporary() {
    if (_global_ctx_is_temporary) {
        return defer_t{_global_ctx_noop};
    }

    _global_ctx_is_temporary = true;
    global_ctx = &_temp_ctx;

    return defer_t{global_ctx_set_default};
}

#define global_ctx_set_scope_temporary()                                               \
    auto UNIQUE_VARIABLE_NAME(scoped_temporary) = global_ctx_set_temporary()
