#pragma once

#include "src/alloc_ctx.hpp"
#include "src/datastructures.hpp"
#include "src/util.hpp"
#include "stringzilla.h"
#include <compare>
#include <stdio.h>

// so annoyed at needing this for the forward definition of std::hash...
#include <functional>

static sz_memory_allocator_t sz_allocator{
    .allocate = [](size_t bytes, void *) { return global_ctx->alloc(bytes); },
    .free = [](void *ptr, size_t, void *) { return global_ctx->free(ptr); },
};

template <typename char_type> struct _string_slice;

using string_view = _string_slice<const char>;
using string_span = _string_slice<char>;

template <typename char_type> struct _string_slice {
    char_type *start;
    size_t length;

    _string_slice() : start(nullptr), length(0) {}
    _string_slice(char_type *s) : start(s), length(strlen(s)) {}
    _string_slice(char_type *s, size_t len) : start(s), length(len) {}

    // some helper functions to avoid needing to get _inner
    char_type *data() const { return start; }
    char_type *begin() const { return start; }
    char_type *end() const { return start + length; }
    const char_type *cbegin() const { return start; }
    const char_type *cend() const { return start + length; }
    size_t len() const { return length; }
    size_t size() const { return length; }

    char_type &operator[](size_t pos) { return this->data()[pos]; }

    bool operator==(string_view other) const {
        return this->length == other.length && sz_equal(begin(), other.begin(), this->length);
    }
    bool operator!=(string_view other) const { return !operator==(other); }

    operator string_view() { return string_view{start, length}; }
    string_view view() { return string_view(*this); }
};

struct string {
    sz_string_t _inner;

    string_view view() const { return string_view(*this); }
    string_span span() const { return string_span(*this); }

    char *data() const { return span().begin(); }
    char *begin() const { return span().begin(); }
    char *end() const { return span().end(); }
    const char *cbegin() const { return view().begin(); }
    const char *cend() const { return view().end(); }
    size_t len() const { return view().len(); }
    size_t size() const { return view().size(); }

    bool operator==(string_view other) { return view() == other; }
    bool operator!=(string_view other) { return view() != other; }
    bool operator==(string other) { return view() == other.view(); }
    bool operator!=(string other) { return view() != other.view(); }

    char *resize(size_t length) {
        if (len() >= length) {
            return data();
        }

        return sz_string_reserve(&_inner, length, &sz_allocator);
    }

    operator string_view() const {
        char *ptr;
        size_t length;

        sz_string_range(&_inner, &ptr, &length);

        return string_view{ptr, length};
    }

    operator string_span() const {
        char *ptr;
        size_t length;

        sz_string_range(&_inner, &ptr, &length);

        return string_span{ptr, length};
    }

    char &operator[](size_t pos) { return this->data()[pos]; }
};

static string string_make(string_view view) {
    string str;

    char *ptr = sz_string_init_length(&str._inner, view.len(), &sz_allocator);
    sz_copy(ptr, view.data(), view.len());

    return str;
}

static string string_make(string str) { return string_make(str.view()); }

static string string_make(const char *s, size_t length) {
    return string_make(string_view(s, length));
}

static string string_make(const char *s) {
    return string_make(string_view(s, strlen(s)));
}
static string string_make() {
    string str;

    sz_string_init(&str._inner);

    return str;
}

struct string_with_begin_ptr {
    string str;
    char *ptr;
};

static string_with_begin_ptr string_make(size_t length) {
    string str;

    char *ptr = sz_string_init_length(&str._inner, length, &sz_allocator);

    return {str, ptr};
}

static void string_destroy(string *str) {
    sz_string_free(&str->_inner, &sz_allocator);
}

static force_inline bool str_starts_with(string_view str, string_view prefix) {
    return str.len() >= prefix.len() &&
           sz_equal(str.begin(), prefix.begin(), prefix.len());
}

static force_inline bool str_ends_with(string_view str, string_view prefix) {
    return str.len() >= prefix.len() &&
           sz_equal(str.end() - prefix.len(), prefix.begin(), prefix.len());
}

static string str_concat(string_view *views, size_t length) {
    string str;

    size_t char_sum = 0;
    for (size_t i = 0; i < length; i++) {
        char_sum += views[i].len();
    }

    char *to = sz_string_init_length(&str._inner, char_sum, &sz_allocator);
    for (size_t i = 0; i < length; i++) {
        const char *from = views[i].data();
        size_t from_len = views[i].len();
        sz_copy(to, from, from_len);
        to += from_len;
    }

    return str;
}

static string str_join(string_view join_arg, string_view const *views, size_t length) {
    string str;

    if (length == 0) {
        sz_string_init(&str._inner);
        return str;
    }

    size_t char_sum = join_arg.len() * (length - 1);
    for (size_t i = 0; i < length; i++) {
        char_sum += views[i].len();
    }

    char *to = sz_string_init_length(&str._inner, char_sum, &sz_allocator);
    sz_copy(to, views[0].data(), views[0].len());
    to += views[0].len();

    for (size_t i = 1; i < length; i++) {
        sz_copy(to, join_arg.data(), join_arg.len());
        to += join_arg.len();

        sz_copy(to, views[i].data(), views[i].len());
        to += views[i].len();
    }

    return str;
}

struct string_builder {
    vector<string_view> vec;
    size_t byte_length;

    void append(string_view view) {
        vec.append(view);
        byte_length += view.len();
    }

    string str() {
        auto [str, start] = string_make(this->byte_length);

        char *writer = start;
        for (string_view view : this->vec) {
            memcpy(writer, view.begin(), view.len());
            writer += view.len();
        }

        return str;
    }

    operator string() {
        return this->str();
    }
};

static string_builder string_builder_make(size_t estimated_length) {
    string_builder builder;

    builder.vec = vector_make<string_view>(estimated_length);
    builder.byte_length = 0;

    return builder;
}

static void string_builder_destroy(string_builder * builder) {
    vector_destroy(&builder->vec);
}

#define str_concat_inline(...)                                                         \
    str_concat((string_view[]){__VA_ARGS__}, ARRAY_LEN(((string_view[]){__VA_ARGS__})))

#define str_join_inline(joined_by, ...)                                                \
    str_join(joined_by, (string_view[]){__VA_ARGS__},                                  \
             ARRAY_LEN(((string_view[]){__VA_ARGS__})))

template <> struct std::hash<::string_view> {
    size_t operator()(const ::string_view &str) const {
        size_t hashed = sz_hash(str.begin(), str.len());

        return hashed;
    }
};

template <> struct std::hash<::string> {
    size_t operator()(const ::string &str) const {
        size_t hashed = sz_hash(str.begin(), str.len());

        return hashed;
    }
};
