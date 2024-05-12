#pragma once

#include "src/util.hpp"

template <typename T>
struct promise;

template <typename T = void>
struct promise {
    void (*fn)(promise<T> *promise);
    T *user_data = nullptr;
    promise *then = nullptr;

    void operator()() { this->fn(this); }
};

