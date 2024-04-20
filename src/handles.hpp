#pragma once

#include "./alloc_ctx.hpp"
#include "./bitfield.hpp"
#include "./bits.hpp"
#include "./span.hpp"
#include "./util.hpp"
#include "bits.hpp"
#include <climits>
#include <cstdint>
#include <cstdlib>

#include <nmmintrin.h>

typedef u16 handle_index_t;
#ifdef SLOW
typedef u32 handle_t;
#define HANDLE_TYPE_BITS 4
#else
typedef u16 handle_t;
#define HANDLE_TYPE_BITS 0
#endif

struct handle_pool_t {
    span<u64> available_slots;
    u16 slot_cursor;

#ifdef SLOW
    span<handle_t> handle_ids;
    u8 bits_used;
    u8 type;
#endif
};

static void handle_pool_create(handle_pool_t *pool, u16 number_members, u8 type) {
    // i don't want to deal with the additional logic this would require for
    // checking available_slots
    assert(number_members % 64 == 0);

    size_t slot_len = (number_members / 64);
    size_t slot_size = slot_len * sizeof(u64);
    u64 *allocation = (u64 *)global_ctx->alloc(slot_size);
    memset(allocation, 0xff, slot_size);

    pool->available_slots = {allocation, slot_len};
    pool->slot_cursor = 0;

#ifdef SLOW
    assert(most_significant_bit(type) <= HANDLE_TYPE_BITS);

    pool->type = type;
    handle_t *id_allocations =
        (handle_t *)global_ctx->calloc(number_members, sizeof(handle_t));
    pool->handle_ids = {id_allocations, number_members};

    pool->bits_used = most_significant_bit(number_members - 1);
#endif
}

static void handle_pool_destroy(handle_pool_t *pool) {
    global_ctx->free((void *)pool->available_slots.ptr);
#ifdef SLOW
    global_ctx->free((void *)pool->handle_ids.ptr);
#endif
    *pool = {};
}

static handle_index_t handle_index_get(handle_pool_t *pool, handle_t handle) {
#ifdef SLOW
    handle_t type_mask = fill_least_bits(HANDLE_TYPE_BITS) << pool->bits_used;
    handle_t type = (handle_t(pool->type)) << pool->bits_used;

    assert((handle & type_mask) == type);

    handle_t index_mask = fill_least_bits(pool->bits_used);
    handle_t index = handle & index_mask;

    u8 id_shift_size = pool->bits_used + HANDLE_TYPE_BITS;

    handle_t id = pool->handle_ids[index] << id_shift_size;
    handle_t id_mask = handle_t(-1) << id_shift_size;

    assert((handle & id_mask) == id);

    return index;
#else
    return handle;
#endif
}

static handle_t handle_index_create(handle_pool_t *pool) {
    u16 len = pool->available_slots.len;
    u8 bit = -1;

    for (u16 i = 0; i < len; i++) {
        bit = least_significant_bit(pool->available_slots[pool->slot_cursor]);
        if (bit < 64) {
            break;
        }
        pool->slot_cursor += 1;
        if (pool->slot_cursor >= pool->available_slots.len) {
            pool->slot_cursor = 0;
        }
    }
    assert(bit < 64);

    pool->available_slots[pool->slot_cursor] ^= 1ll << bit;
    handle_t index = pool->slot_cursor * 64 + bit;

#ifdef SLOW
    pool->handle_ids[index] += 1;

    handle_t type = pool->type << pool->bits_used;
    handle_t id = pool->handle_ids[index] << (pool->bits_used + HANDLE_TYPE_BITS);
    return index | type | id;
#else
    return index;
#endif
}

static void handle_index_destroy(handle_pool_t *pool, handle_t handle) {
    handle_index_t index = handle_index_get(pool, handle);

    size_t cursor = index / 64;
    size_t bit = index % 64;

    u64 &slots = pool->available_slots[cursor];
    assert((slots & (1ll << bit)) == 0);
    slots |= 1ll << bit;
}
