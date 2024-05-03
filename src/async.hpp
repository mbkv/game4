#pragma once

struct promise;

typedef void (*async_fn)(promise *this_fn);

struct promise {
    void *user_data;
    async_fn current;
    promise *then;

    promise(async_fn fn) : user_data(nullptr), current(fn), then(nullptr) {}
    promise(void *user_data, async_fn fn)
        : user_data(user_data), current(fn), then(nullptr) {}

    void operator()() { this->current(this); }
};
