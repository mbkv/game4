#pragma once

#include "./util.hpp"
#include <cstdint>
#include <malloc.h>
#include <stdio.h>

struct allocator_t {
    void *(*alloc)(size_t);
    void *(*calloc)(size_t, size_t);
    void *(*realloc)(void *, size_t);
    void (*free)(void *);
};

#define MAX_TEMP_ALLOC_SIZE (1024 * 1024 * 16)

struct arena_t {
    uintptr_t _start;
    uintptr_t _arena;
    size_t arena_size;
};


static arena_t arena_make(size_t bytes_requested) {
    arena_t arena {};
    arena._arena = (uintptr_t) malloc(bytes_requested);
    arena._start = arena._arena;
    arena.arena_size = bytes_requested;

    return arena;
}

static void *arena_alloc(arena_t *arena, size_t bytes_requested) {
    // assert there's even size to give back;
    const size_t total_size_after_allocation = arena->_arena - arena->_start + bytes_requested;
    if (total_size_after_allocation > arena->arena_size) {
        fprintf(stderr, "Arena allocator %p is out of memory", arena);
        return nullptr;
    }

    // we reserve an additional size_t for bytes_requested which is needed for
    // realloc
    arena->_arena += sizeof(size_t);
    // we serve 16 bytes aligned all the time
    arena->_arena = roundup_by_power2(arena->_arena, 16);
    assert(arena->_arena % 16 == 0);
    void *return_value = (void *)arena->_arena;
    size_t *size_value = ((size_t *)arena->_arena) - 1;
    *size_value = bytes_requested;
    arena->_arena += bytes_requested;
    return return_value;
}

static void *arena_calloc(arena_t *arena, size_t number_members, size_t size) {
    u64 total_size = number_members * size;
    void *ptr = arena_alloc(arena, total_size);

    memset(ptr, 0, total_size);

    return ptr;
}

static void arena_freeall(arena_t *arena) {
    arena->_arena = arena->_start;
}

static void *arena_realloc(arena_t * arena, void *ptr, size_t size) {
    if (size == 0) {
        // don't need to free since this is an arena
        return nullptr;
    } else if (ptr == nullptr) {
        return arena_alloc(arena, size);
    }
    size_t *old_size = ((size_t *)ptr) - 1;
    if (*old_size >= size) {
        return ptr;
    }
    // gotta allocate a new array
    void *new_ptr = arena_alloc(arena, size);
    void *new_ptr_end = mempcpy(new_ptr, ptr, *old_size);
    memset(new_ptr_end, 0, size - *old_size);
    return new_ptr;
}

static void arena_debug(arena_t *arena) {
    printf("temp_alloc allocated size: %ld\n", arena->_arena - arena->_start);
}

static arena_t global_arena_ctx = arena_make(MAX_TEMP_ALLOC_SIZE);

static void *global_arena_alloc(size_t bytes_requested) {
    return arena_alloc(&global_arena_ctx, bytes_requested);
}

static void *global_arena_calloc(size_t number_members, size_t size) {
    return arena_calloc(&global_arena_ctx, number_members, size);
}

// ~some~ most apis need a free along side an alloc
static void global_arena_free(void * ptr) {

}

static void global_arena_freeall() { arena_freeall(&global_arena_ctx); }

static void *global_arena_realloc(void *ptr, size_t size) {
    return arena_realloc(&global_arena_ctx, ptr, size);
}

static void global_arena_debug()
{
    arena_debug(&global_arena_ctx);
}

static allocator_t temp_allocator{global_arena_alloc, global_arena_calloc,
                                  global_arena_realloc, global_arena_free};

struct global_context : allocator_t {
    allocator_t temp;
};

const global_context _alloc_ctx{{malloc, calloc, realloc, free}, temp_allocator};
const global_context _temp_ctx{{temp_allocator}, temp_allocator};

static bool _global_ctx_is_temporary = false;
static global_context const *global_ctx = &_alloc_ctx;

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
