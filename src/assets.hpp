#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "./alloc_ctx.hpp"
#include "./os.hpp"
#include "./string.hpp"
#include "./util.hpp"

#define TINYOBJ_MALLOC global_ctx->alloc
#define TINYOBJ_REALLOC global_ctx->realloc
#define TINYOBJ_CALLOC global_ctx->calloc
#define TINYOBJ_FREE global_ctx->free
#include "tinyobj_loader_c.h"

#define STBI_MALLOC global_ctx->alloc
#define STBI_REALLOC global_ctx->realloc
#define STBI_FREE global_ctx->free
#include "stb_image.h"

static void _tinyobj_file_reader_impl(void *ctx, const char *filename, int is_mtl,
                                     const char *obj_filename, char **buf,
                                     size_t *len) {
    auto defer_lock = global_ctx_set_temporary();

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

void asset_cleanup(asset_tinyobj &asset) {
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

asset_image asset_image_load_rgb(const char *filename) {
    auto defer_lock = global_ctx_set_temporary();
    str file = read_entire_file(filename);
    assert(file.s);

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
