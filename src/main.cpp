#include "stdio.h"
#include "src/assets.hpp"
#include "src/os.hpp"
#include "src/util.hpp"

int main() {
    const char *files[] = {
        resource_get_path("box texture.png"),
        resource_get_path("shaders/game.fs"),
        resource_get_path("shaders/game.vs"),
        resource_get_path("shaders/ui.fs"),
        resource_get_path("shaders/ui.vs"),
    };

    assets_init(files, ARRAY_LEN(files), [](){
        printf("Finished downloading\n");
    });
}

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
    handle_pool_create(&pool, 32768, 0b0111);
    }

    handle_t handles[32768] = {};

    rand64_state rng_state{0xdeadbeef};
    u64 *rngs = (u64 *)malloc(sizeof(u64) * 1 << 20);;

    {
        profile(1 << 20);

        for( int i = 0; i < (1 << 20); i++) {
            rngs[i] = rand64(&rng_state);
        }
    }
    {
        for (int i = 0; i < (1 << 20); i++) {
            printf("%lu\n", rngs[i]);
        }
    }

    {
        profile(32768);

        for (int i = 0; i < 32768; i++) {
            handles[i] = handle_index_create(&pool);
        }
    }
    {
        for (int i = 0; i < 32768; i++) {
            printBinary(handles[i]);
            putchar(' ');

            handle_index_t index = handle_index_get(&pool, handles[i]);
            printBinary(index);
            putchar('\n');
        }
    }

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
