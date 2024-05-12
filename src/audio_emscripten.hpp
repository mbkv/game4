#pragma once

#include "src/handles.hpp"
#include "src/string.hpp"
#include "src/util.hpp"
#include <emscripten/emscripten.h>

typedef u8 audio_id;

static handle_pool_t audio_handle_pool = handle_pool_create(32, TYPE_AUDIO);

EM_JS(bool, _audio_init, (const char *, handle_t id), {
    
});

static handle_t audio_init(string_view file) {
    handle_t handle = handle_index_create(&audio_handle_pool);

}
