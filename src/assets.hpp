#pragma once

#include "src/emscripten.hpp"
#include "src/os.hpp"

#if 0
#include <stdio.h>
#include <stdlib.h>

#include "emscripten/fetch.h"
#include "src/alloc_ctx.hpp"
#include "src/emscripten.hpp"
#include "src/handles.hpp"
#include "src/string.hpp"
#include "src/util.hpp"

#define FAST_OBJ_REALLOC global_ctx->realloc
#define FAST_OBJ_FREE global_ctx->free
#include "vendor/fast_obj.h"

#define TINYOBJ_MALLOC global_ctx->alloc
#define TINYOBJ_REALLOC global_ctx->realloc
#define TINYOBJ_CALLOC global_ctx->calloc
#define TINYOBJ_FREE global_ctx->free
#include "vendor/tinyobj_loader_c.h"

#define STBI_MALLOC global_ctx->alloc
#define STBI_REALLOC global_ctx->realloc
#define STBI_FREE global_ctx->free
#include "vendor/stb_image.h"

handle_pool_t downloaded_files;

static void _tinyobj_file_reader_impl(void *ctx, const char *filename, int is_mtl,
                                      const char *obj_filename, char **buf,
                                      size_t *len) {
    global_ctx_set_scope_temporary();

    str file = read_entire_file(filename);
    assert(file.s);
    *buf = file.s;
    *len = file.len;
}

struct asset_tinyobj {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes;
    size_t num_shapes;
    tinyobj_material_t *materials;
    size_t num_materials;
};

static asset_tinyobj asset_tinyobj_parse(const char *filename) {
    asset_tinyobj asset;
    tinyobj_parse_obj(&asset.attrib, &asset.shapes, &asset.num_shapes, &asset.materials,
                      &asset.num_materials, "res/box.obj", _tinyobj_file_reader_impl,
                      nullptr, 0);

    return asset;
}

/* fastObjMesh* thing(const char* filename) { */
/* 	return fast_obj_read(filename); */
/* } */

static void asset_cleanup(asset_tinyobj &asset) {
    tinyobj_attrib_free(&asset.attrib);
    tinyobj_shapes_free(asset.shapes, asset.num_shapes);
    tinyobj_materials_free(asset.materials, asset.num_materials);
}

struct asset_image {
    u8 *data;
    int w;
    int h;
    int channels;
};

static asset_image asset_image_load_rgb(const char *filename) {
    global_ctx_set_scope_temporary();
    str file = read_entire_file(filename);
    assert(file.s);
    stbi_set_flip_vertically_on_load(true);

    int x;
    int y;
    int channels_in_file;
    int desired_channels = 3;
    u8 *image = stbi_load_from_memory((u8 *)file.s, file.len, &x, &y, &channels_in_file,
                                      desired_channels);
    return {
        .data = image,
        .w = x,
        .h = y,
        .channels = desired_channels,
    };
}
#endif

static size_t running_downloads = 0;

static void assets_init(const char **assets_to_download, size_t asset_len,
                        next_fn next) {
    file_reader reader = [](const char *response, void *user_data) {
        running_downloads -= 1;
        if (running_downloads == 0) {
            next_fn next = (next_fn)user_data;
            next();
        }
    };
    error_handler handler = [](void *) {
        fprintf(stderr, "Error file not found\n");
        exit(1);
    };
    for (size_t i = 0; i < asset_len; i++) {
        running_downloads += 1;
        read_entire_file_async(assets_to_download[i], reader, handler, (void *)next);
    }
}
