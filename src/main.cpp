#if 1
#include "src/assets.hpp"
#include "src/async.hpp"
#include "src/gl.hpp"
#include "src/os.hpp"
#include "vendor/linalg.h"
#include <emscripten.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>
#include <src/string.hpp>
#include <src/util.hpp>
#include <webgl/webgl2.h>

size_t frame_counter = 0;
f32 start_time = -1;

typedef linalg::vec<f32, 2> vec2;
typedef linalg::vec<f32, 3> vec3;
typedef linalg::mat<f32, 4, 4> mat4;

vec2 window_size{1920, 1080};
mat4 camera = linalg::lookat_matrix(vec3{-3, 3, -3}, vec3{0, 0, 0}, vec3{0, 1, 0});
mat4 position = linalg::identity_t{4};
mat4 projection =
    linalg::perspective_matrix(to_radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl;

EM_JS(f32, canvas_set_size_properly, (const char *s, f32 x, f32 y), {
    const canvas = document.querySelector(UTF8ToString(s, Infinity));
    canvas.width = x;
    canvas.height = y;
    canvas.style.width = x / window.devicePixelRatio + 'px';
    canvas.style.height = y / window.devicePixelRatio + 'px';
})

EM_JS(void, browser_alert, (const char *s), {alert(UTF8ToString(s, Infinity))})

static const string_view canvas_selector{"#canvas"};

static bool make_window() {
    canvas_set_size_properly(canvas_selector.begin(), window_size.x, window_size.y);

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;

    webgl = emscripten_webgl_create_context(canvas_selector.begin(), &attrs);
    if (!webgl) {
        return false;
    }
    emscripten_webgl_make_context_current(webgl);

    return true;
}

gl_program game_shader;
gl_program ui_shader;

gl_draw_object square;
gl_draw_object cube;
GLuint cube_texture;

static EM_BOOL loop(double time, void *userData) {
    global_arena_freeall();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(game_shader.program);
    glBindTexture(GL_TEXTURE_2D, cube_texture);

    glUniformMatrix4fv(game_shader.uniforms.at("uProjectionMatrix"), 1, GL_FALSE,
                       &projection[0][0]);
    glUniformMatrix4fv(game_shader.uniforms.at("uViewMatrix"), 1, GL_FALSE,
                       &camera[0][0]);
    glUniformMatrix4fv(game_shader.uniforms.at("uWorldMatrix"), 1, GL_FALSE,
                       &position[0][0]);
    gl_asset_draw(&cube);

    glUseProgram(ui_shader.program);
    glUniform2f(ui_shader.uniforms.at("uScreenSize"), window_size.x, window_size.y);
    glBindTexture(GL_TEXTURE_2D, cube_texture);
    gl_asset_draw(&square);

    return true;
}

static EM_BOOL loop_loading_check(double time, void *userData) {
    global_arena_freeall();
    if (total_running_downloads != 0) {
        return true;
    }

    game_shader =
        gl_shader_load(res_path("shaders/game.vs"), res_path("shaders/game.fs"));
    ui_shader = gl_shader_load(res_path("shaders/ui.vs"), res_path("shaders/ui.fs"));
    cube = gl_asset_load(res_path("box textured.obj"));

    square = gl_ui_rect_load({100., 100., 200., 200.}, {0.0, 0.0, 1.0, 1.0});

    asset_image img = asset_image_load_rgb(res_path("box texture.png"));
    cube_texture = gl_texture_load(&img);

    emscripten_request_animation_frame_loop(loop, nullptr);

    return false;
}

int main() {
    string_view files[] = {
        (res_path("box texture.png")),  (res_path("box textured.obj")),
        (res_path("box textured.mtl")), (res_path("shaders/game.fs")),
        (res_path("shaders/game.vs")),  (res_path("shaders/ui.fs")),
        (res_path("shaders/ui.vs"))};

    if (make_window()) {
        assets_init(files, ARRAY_LEN(files), nullptr);
        gl_init();
        glViewport(0, 0, 1920, 1080);
        emscripten_request_animation_frame_loop(loop_loading_check, nullptr);
    } else {
        browser_alert("Could not make webgl window");
    }
}
#endif

#if 0
#include "src/handles.hpp"
#include "src/profiler.hpp"
#include "src/rand.hpp"
#include <cstdlib>

template<typename T>
void printBinary(T value) {
    static_assert(std::is_unsigned<T>::value, "T must be an unsigned integer type");
    unsigned shift = sizeof(T) * 8 - 1;
    char buffer[sizeof(T) * 8 + 1];
    buffer[sizeof(T) * 8] = '\0';
    while (shift > 0) {
        buffer[shift] = (value & 1) ? '1' : '0';
        value >>= 1;
        --shift;
    }
    buffer[shift] = (value & 1) ? '1' : '0';
    printf("%s", buffer);
}

int main() {
    handle_pool_t pool;
    {
    profile(1);
    pool = handle_pool_create(32768, 0b0111);
    }

    handle_t handles[32768] = {};

    rand64_state rng_state{0xdeadbeef};
    u64 *rngs = (u64 *)ctx->alloc(sizeof(u64) * 1 << 14);;

    {
        profile(1 << 14);

        for( int i = 0; i < (1 << 14); i++) {
            rngs[i] = rand64(&rng_state);
        }
    }
#if 0
    {
        for (int i = 0; i < (1 << 14); i++) {
            printf("%llu\n", rngs[i]);
        }
    }
#endif

    {
        profile(32768);

        for (int i = 0; i < 32768; i++) {
            handles[i] = handle_index_create(&pool);
        }
    }
#if 0
    {
        for (int i = 0; i < 32768; i++) {
            printBinary(handles[i]);
            putchar(' ');

            handle_index_t index = handle_index_get(&pool, handles[i]);
            printBinary(index);
            putchar('\n');
        }
    }
#endif

    {
        profile(32768 * 32);

        for (int j = 0; j < 32; j++) {
            for (int i = 0; i < 32768; i++) {
                handle_index_t index = handle_index_get(&pool, handles[i]);

                assert(index < 32768);
            }
        }
    }

    {
        profile(32768);

        for (int i = 0; i < 32768; i++) {
            handle_index_destroy(&pool, handles[i]);
        }
    }
    {
        profile(32768 * 128);
        for (int j = 0; j < 128; j++) {

            for (int i = 0; i < 32768; i++) {
                handles[i] = handle_index_create(&pool);
            }
            for (int i = 0; i < 32768; i++) {
                handle_index_destroy(&pool, handles[i]);
            }
        }
    }

    {
        profile(1 << 20);
        for (int i = 0; i < (1 << 20); i++) {
            handle_t handle = handle_index_create(&pool);

            handle_index_destroy(&pool, handle);
        }
    }
    return EXIT_SUCCESS;
}
#endif

#if 0
#include "src/alloc_ctx.hpp"
#include "src/assets.hpp"
#include "src/gl.hpp"
#include "src/handles.hpp"
#include "src/util.hpp"
#include "vendor/linalg.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Window *win;
SDL_GLContext context;

typedef linalg::vec<f32, 2> vec2;
typedef linalg::vec<f32, 3> vec3;
typedef linalg::mat<f32, 4, 4> mat4;

static bool make_window(vec2 window_size) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED, window_size[0], window_size[1],
                           SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS |
                               SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
    if (win == nullptr) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    context = SDL_GL_CreateContext(win);
    if (context == nullptr) {
        fprintf(stderr, "SDL_GL_CreateContext Error: %s\n", SDL_GetError());
        return false;
    }
    glewExperimental = true;
    GLenum glew_err = glewInit();
    if (glew_err != GLEW_OK) {
        fprintf(stderr, "glewInit Error: %s\n", glewGetErrorString(glew_err));
        return false;
    }

    return true;
}

mat4 camera = linalg::lookat_matrix(vec3{-3, 3, -3}, vec3{0, 0, 0}, vec3{0, 1, 0});
mat4 position = linalg::identity_t{4};
mat4 projection =
    linalg::perspective_matrix(to_radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

gl_draw_object asset_box;
GLuint asset_texture;

GLuint program;
Uniforms program_uniforms;
SDL_Event evt;
bool running = true;

void tick() {
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                running = false;
            break;
        }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, asset_texture);
    glUniformMatrix4fv(program_uniforms.at("uProjectionMatrix"), 1, GL_FALSE,
                       &projection[0][0]);
    glUniformMatrix4fv(program_uniforms.at("uViewMatrix"), 1, GL_FALSE,
                       &camera[0][0]);
    glUniformMatrix4fv(program_uniforms.at("uWorldMatrix"), 1, GL_FALSE,
                       &position[0][0]);
    gl_asset_draw(&asset_box);

    global_arena_freeall();
    SDL_GL_SwapWindow(win);
}

int main() {
    vec2 window_size{1920, 1080};
    if (!make_window(window_size)) {
        return EXIT_FAILURE;
    }

    defer(SDL_Quit());

    gl_init();

    asset_box = gl_asset_load("box textured.obj");
    program = gl_shader_load("shaders/game.vs", "shaders/game.fs");
    gl_get_all_uniform_locations(program, program_uniforms);

    glUseProgram(program);
    asset_image img = asset_image_load_rgb("box texture.png");
    asset_texture = gl_texture_load(&img);

    global_arena_debug();

#ifdef EMSCRIPTEN
#else
    while (running) {
    }
#endif

    return EXIT_SUCCESS;
}
#endif
