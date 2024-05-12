#pragma once

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "src/alloc_ctx.hpp"
#include "src/string.hpp"
#include <emscripten/fetch.h>
#include <functional>
#include <stdio.h>
#include <unordered_map>

typedef void (*file_reader)(string str, string_view filename, void *user_data);
typedef void (*error_handler)(string_view filename, void *user_data);

struct _read_entire_file_async_user_data {
    file_reader on_success;
    error_handler on_error;
    const char *filename;
    void *user_data;
};

static void _read_entire_file_async_success(emscripten_fetch_t *fetch) {
    auto *my_data = (_read_entire_file_async_user_data *)fetch->userData;

    void *user_data = my_data->user_data;
    file_reader on_success = my_data->on_success;

    const char *response_body = fetch->data;
    size_t response_len = fetch->numBytes;
    string response = string_make(response_body, response_len);

    ctx->free(my_data);
    on_success(response, my_data->filename, user_data);
    emscripten_fetch_close(fetch);
}

static void _read_entire_file_async_error(emscripten_fetch_t *fetch) {
    auto *my_data = (_read_entire_file_async_user_data *)fetch->userData;

    void *user_data = my_data->user_data;
    error_handler on_error = my_data->on_error;

    ctx->free(my_data);

    on_error(my_data->filename, user_data);
    emscripten_fetch_close(fetch);
}

static void read_entire_file_async(const char *filename, file_reader on_success,
                                   error_handler on_error, void *user_data = nullptr) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = _read_entire_file_async_success;
    attr.onerror = _read_entire_file_async_error;

    auto *my_data = allocate<_read_entire_file_async_user_data>();
    my_data->on_success = on_success;
    my_data->on_error = on_error;
    my_data->user_data = user_data;
    my_data->filename = filename;
    attr.userData = my_data;

    emscripten_fetch(&attr, filename);
}

static void read_entire_file_async_free(string_view ptr) {}

#define res_path(file) ("/res/" file)

typedef double high_frequency_timer_t;

static high_frequency_timer_t get_high_frequency_time() {
    // assuming 5gigahert timer
    return emscripten_performance_now() * 5'000'000.0;
}

#endif
