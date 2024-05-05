#pragma once

#include "src/alloc_ctx.hpp"
#include "src/util.hpp"

template <typename T> struct vector {
    T *data = nullptr;

    size_t len = 0;
    size_t capacity = 0;

    T *begin() const { return data; }
    T *end() const { return data + len; }
    const T *cbegin() const { return data; }
    const T *cend() const { return data + len; }
    size_t size() const { return len; }

    void append(T value) {
        if (this->len >= this->capacity) {
            size_t new_capacity = this->capacity * 2;
            this->capacity = new_capacity;
            this->data = (T *)global_ctx->realloc(this->data, sizeof(T) * new_capacity);
        }

        this->data[this->len] = value;
        this->len += 1;
    }

    T pop() {
        this->len -= 1;
        return this->data[this->len];
    }

    void remove(size_t i = 0) {
        this[i] = this->pop();
    }

    T &operator[](size_t i) {
        assert(i < this->len);
        return this->data[i];
    }
};

template <typename T> vector<T> vector_make(size_t amount) {
    vector<T> vec;
    vec.data = (T *)global_ctx->alloc(amount * sizeof(T));
    vec.len = 0;
    vec.capacity = amount;

    return vec;
};

template <typename T> void vector_destroy(vector<T> *vec) {
    global_ctx->free(vec->data);
    *vec = {};
}

template <typename T>
struct span {
    T *ptr;
    size_t len;

    T &operator[](size_t i) {
        assert(i < len);

        return ptr[i];
    }
};

