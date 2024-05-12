#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "src/alloc_ctx.hpp"
#include "src/async.hpp"
#include "src/handles.hpp"
#include "src/os.hpp"
#include "src/string.hpp"
#include "src/util.hpp"

#define FAST_OBJ_REALLOC ctx->realloc
#define FAST_OBJ_FREE ctx->free
#include "vendor/fast_obj.h"

#define STBI_MALLOC ctx->alloc
#define STBI_REALLOC ctx->realloc
#define STBI_FREE ctx->free
#include "vendor/stb_image.h"

static handle_pool_t textures = handle_pool_create(1024, TYPE_TEXTURES);
static handle_pool_t models = handle_pool_create(1024, TYPE_MODELS);

static std::unordered_map<string_view, string> downloaded_files;

static string &asset_downloaded_file_get(string_view filename) {
    auto found = downloaded_files.find(filename);
    if (found == downloaded_files.end()) {
        fprintf(stderr, "File '%s' not downloaded\n", filename.begin());
        exit(1);
    }

    return found->second;
}

#if 0
#define TINYOBJ_MALLOC ctx->alloc
#define TINYOBJ_REALLOC ctx->realloc
#define TINYOBJ_CALLOC ctx->calloc
#define TINYOBJ_FREE ctx->free
#include "vendor/tinyobj_loader_c.h"

static void _tinyobj_file_reader_impl(void *ctx, const char *filename, int is_mtl,
                                      const char *obj_filename, char **buf,
                                      size_t *len) {
    ctx_temp_lock();

    string_span file = asset_downloaded_file_get(filename);

    *buf = file.begin();
    *len = file.len();
}

struct asset_tinyobj {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes;
    size_t num_shapes;
    tinyobj_material_t *materials;
    size_t num_materials;
};

static asset_tinyobj asset_tinyobj_parse(string_view filename) {
    asset_tinyobj asset;
    tinyobj_parse_obj(&asset.attrib, &asset.shapes, &asset.num_shapes, &asset.materials,
                      &asset.num_materials, filename.begin(), _tinyobj_file_reader_impl,
                      nullptr, 0);

    return asset;
}

static void asset_cleanup(asset_tinyobj *asset) {
    tinyobj_attrib_free(&asset->attrib);
    tinyobj_shapes_free(asset->shapes, asset->num_shapes);
    tinyobj_materials_free(asset->materials, asset->num_materials);
}

#endif

struct _fast_obj_callback_state {
    string file;
    size_t reading_index;
};

const fastObjCallbacks fast_obj_callbacks{
    .file_open =
        [](const char *path, void *user_data) {
            auto *state = allocate<_fast_obj_callback_state>();

            state->file = asset_downloaded_file_get(path);

            return (void *)state;
        },
    .file_close = [](void *file, void *user_data) {
        ctx->free(file);
    },
    .file_read =
        [](void *file, void *dst, size_t bytes, void *user_data) {
            _fast_obj_callback_state *state = (_fast_obj_callback_state *)file;
            size_t bytes_to_write = state->file.len() - state->reading_index;

            char *string = state->file.begin() + state->reading_index;
            sz_copy((char *)dst, string, bytes_to_write);

            state->reading_index += bytes_to_write;

            return bytes_to_write;
        },
    .file_size =
        [](void *file, void *user_data) {
            _fast_obj_callback_state *state = (_fast_obj_callback_state *)file;
            return state->file.len();
        }};

static fastObjMesh *asset_fastobj_parse(string_view filename) {
    return fast_obj_read_with_callbacks(filename.begin(), &fast_obj_callbacks, nullptr);
}

static void asset_cleanup(fastObjMesh *mesh) { fast_obj_destroy(mesh); }

struct asset_image {
    u8 *data;
    int w;
    int h;
    int channels;
};

static asset_image asset_image_load_rgb(string_view filename) {
    ctx_temp_lock();
    string file = asset_downloaded_file_get(filename);
    stbi_set_flip_vertically_on_load(true);

    int x;
    int y;
    int channels_in_file;
    int desired_channels = 3;
    u8 *image = stbi_load_from_memory((u8 *)file.data(), file.len(), &x, &y,
                                      &channels_in_file, desired_channels);
    return {
        .data = image,
        .w = x,
        .h = y,
        .channels = desired_channels,
    };
}

static void asset_image_destroy(asset_image *img) {
    stbi_image_free(img->data);
    *img = {};
}

struct _assets_download_data {
    size_t running_downloads = 0;
    promise<void> *next;
};

static size_t total_running_downloads = 0;

static void asset_process(string_view filename, string file) {}

static void assets_download(string_view *assets_to_download, size_t asset_len,
                            promise<void> *next) {
    _assets_download_data *my_data = allocate<_assets_download_data>(real_allocator.alloc);
    total_running_downloads += asset_len;
    my_data->running_downloads = asset_len;
    my_data->next = next;

    file_reader reader = [](string response, string_view filename, void *user_data) {
        _assets_download_data *my_data = (_assets_download_data *)user_data;

        downloaded_files[filename] = response;
        asset_process(filename, response);
        total_running_downloads -= 1;
        my_data->running_downloads -= 1;
        if (my_data->running_downloads == 0) {
            if (my_data->next) {
                fprintf(stderr, "%p\n %p\n", my_data->next, my_data->next->fn);
                my_data->next->operator()();
            }
            real_allocator.free(my_data);
        }
    };
    error_handler handler = [](string_view filename, void *) {
        fprintf(stderr, "Error downloading file %s\n", filename.begin());
        // TODO(mbkv) don't do this
        exit(1);
    };
    for (size_t i = 0; i < asset_len; i++) {
        read_entire_file_async(assets_to_download[i].begin(), reader, handler,
                               (void *)my_data);
    }
}

static void assets_init(string_view *assets, size_t assets_len, promise<void> *then) {
    assets_download(assets, assets_len, then);
}
