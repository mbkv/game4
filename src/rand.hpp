#pragma once

#include "./util.hpp"

typedef u64 xorshift64_state;

static u64 xorshift64(xorshift64_state *state)
{
	*state ^= *state << 13;
	*state ^= *state >> 7;
	*state ^= *state << 17;
    return *state;
}
