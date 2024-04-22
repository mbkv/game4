#pragma once

#include <emscripten/emscripten.h>
#ifdef EMSCRIPTEN

#include "src/alloc_ctx.hpp"
#include "src/string.hpp"
#include <emscripten/fetch.h>
#include <functional>
#include <stdio.h>
#include <unordered_map>

typedef void (*file_reader)(const char *file, const char *filename, void *user_data);
typedef void (*error_handler)(const char *filename, void *user_data);

struct _read_entire_file_async_user_data {
    file_reader on_success;
    error_handler on_error;
    void *user_data;
};

static std::unordered_map<void *, emscripten_fetch_t *> _fetch_cleanup_info;

static void _read_entire_file_async_success(emscripten_fetch_t *fetch) {
    _read_entire_file_async_user_data *my_data =
        (_read_entire_file_async_user_data *)fetch->userData;
    emscripten_debugger();

    void *user_data = my_data->user_data;
    file_reader on_success = my_data->on_success;
    const char *response = fetch->data;

    _fetch_cleanup_info[(void *)response] = fetch;

    on_success(response, fetch->url, user_data);
}

static void _read_entire_file_async_error(emscripten_fetch_t *fetch) {
    _read_entire_file_async_user_data *my_data =
        (_read_entire_file_async_user_data *)fetch->userData;

    void *user_data = my_data->user_data;
    error_handler on_error = my_data->on_error;
    on_error(fetch->url, user_data);

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

    _read_entire_file_async_user_data *my_data =
        (_read_entire_file_async_user_data *)global_ctx->temp.alloc(
            sizeof(_read_entire_file_async_user_data));
    my_data->on_success = on_success;
    my_data->on_error = on_error;
    my_data->user_data = user_data;
    attr.userData = my_data;

    emscripten_fetch(&attr, filename);
}

static void read_entire_file_async_free(const char *ptr) {
    emscripten_fetch_t *fetch = _fetch_cleanup_info.at((void *)ptr);
    _fetch_cleanup_info.erase((void *) ptr);
    emscripten_fetch_close(fetch);
}

#define resource_get_path(file) ("/res/" file)

#endif
