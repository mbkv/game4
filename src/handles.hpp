#pragma once

#include "src/alloc_ctx.hpp"
#include "src/bits.hpp"
#include "src/span.hpp"
#include "src/util.hpp"
#include "src/bits.hpp"
#include "src/rand.hpp"
#include <climits>
#include <cstdint>
#include <cstdlib>

#include <nmmintrin.h>

typedef u16 handle_index_t;
typedef u8 handle_type_t;
#ifdef SLOW
typedef u32 handle_t;
typedef u16 handle_id_t;
#define HANDLE_TYPE_BITS 4
#define HANDLE_SHIFT_SIZE (16 + HANDLE_TYPE_BITS)
#else
typedef u16 handle_t;
#endif

#define TYPE_TEXTURES 0b0001
#define TYPE_MODELS 0b0010

struct handle_pool_t {
    size_t len;
    span<u64> available_handles;
    u16 handle_cursor;

#ifdef SLOW
    span<handle_id_t> handle_ids;
    handle_t type_shifted;
#endif
};

static handle_pool_t handle_pool_create(u16 number_members, handle_type_t type) {
    handle_pool_t pool;
    // i don't want to deal with the additional logic this would require for
    // checking available_handles
    assert(number_members % 64 == 0);

    size_t handle_len = (number_members / 64);
    size_t handle_size = handle_len * sizeof(u64);
    u64 *handle_allocation = (u64 *)global_ctx->alloc(handle_size);
    memset(handle_allocation, 0xff, handle_size);

    pool.available_handles = {handle_allocation, handle_len};
    pool.handle_cursor = 0;
    pool.len = number_members;

#ifdef SLOW
    assert(most_significant_bit(type) < 4);
    pool.type_shifted = type << 16;

    handle_id_t *id_allocations =
        (handle_id_t *)global_ctx->alloc(number_members * sizeof(handle_id_t));
    u64 *u64_ids = (u64 *)id_allocations;
    size_t u64_ids_len = number_members / (sizeof(u64) / sizeof(handle_id_t));
    rand64_state state = rand64_seed(0);

    for (size_t i = 0; i < u64_ids_len; i++) {
        u64_ids[i] = rand64(&state);
    }
    pool.handle_ids = {id_allocations, number_members};
#endif

    return pool;
}

static void handle_pool_destroy(handle_pool_t *pool) {
    global_ctx->free((void *)pool->available_handles.ptr);
#ifdef SLOW
    global_ctx->free((void *)pool->handle_ids.ptr);
#endif
    *pool = {};
}

static handle_index_t handle_index_get(handle_pool_t *pool, handle_t handle) {
#ifdef SLOW
    handle_t type_mask = fill_least_bits(HANDLE_TYPE_BITS) << 16;

    assert((handle & type_mask) == pool->type_shifted);

    handle_t index_mask = 0xffff;
    handle_t index = handle & index_mask;
    assert(index < pool->len);

    handle_t id = pool->handle_ids[index] << HANDLE_SHIFT_SIZE;
    handle_t id_mask = handle_id_t(-1) << HANDLE_SHIFT_SIZE;

    assert((handle & id_mask) == id);

    return index;
#else
    assert(handle < pool->len);
    return handle;
#endif
}

static handle_t handle_index_create(handle_pool_t *pool) {
    u16 len = pool->available_handles.len;
    u8 bit = -1;

    for (u16 i = 0; i < len; i++) {
        bit = least_significant_bit(pool->available_handles[pool->handle_cursor]);
        if (bit < 64) {
            break;
        }
        pool->handle_cursor += 1;
        if (pool->handle_cursor >= pool->available_handles.len) {
            pool->handle_cursor = 0;
        }
    }
    assert(bit < 64);

    pool->available_handles[pool->handle_cursor] ^= 1ll << bit;
    handle_t index = pool->handle_cursor * 64 + bit;

    assert(index < pool->len);

#ifdef SLOW
    pool->handle_ids[index] += 1;

    handle_t type = pool->type_shifted;
    handle_t id = pool->handle_ids[index] << (HANDLE_SHIFT_SIZE);
    return index | type | id;
#else
    return index;
#endif
}

static void handle_index_destroy(handle_pool_t *pool, handle_t handle) {
    handle_index_t index = handle_index_get(pool, handle);

    size_t cursor = index / 64;
    size_t bit = index % 64;
    u64 bit_mask = 1ll << bit;

    u64 &handle_group = pool->available_handles[cursor];
    assert((handle_group & bit_mask) == 0);
    handle_group |= bit_mask;
}
