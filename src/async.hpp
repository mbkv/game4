#pragma once

struct promise;

typedef void (*async_fn)(promise *this_fn);

struct promise {
    async_fn fn;
    void *user_data = nullptr;
    promise *then = nullptr;

    void operator()() { this->fn(this); }
};
