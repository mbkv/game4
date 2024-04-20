#include "./handles.hpp"
#include "./profiler.hpp"
#include <cstdlib>

int main() {
    handle_pool_t pool;
    handle_pool_create(&pool, 32768, 0b1111);

    handle_t handles[32768] = {};

    {
        profile(32768);

        for (int i = 0; i < 32768; i++) {
            handles[i] = handle_index_create(&pool);
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

#if 0
#include "./alloc_ctx.hpp"
#include "./assets.hpp"
#include "./gl.hpp"
#include "./handles.hpp"
#include "./util.hpp"
#include "linalg.h"
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

int main() {
    vec2 window_size{1920, 1080};
    if (!make_window(window_size)) {
        return EXIT_FAILURE;
    }

    defer(SDL_Quit());

    gl_init();

    mat4 camera = linalg::lookat_matrix(vec3{-3, 3, -3}, vec3{0, 0, 0}, vec3{0, 1, 0});
    ;
    mat4 position = linalg::identity_t{4};
    mat4 projection =
        linalg::perspective_matrix(to_radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

    gl_draw_object asset_box = gl_asset_load("./res/box textured.obj");

    GLuint program = gl_shader_load("./res/shaders/game.vs", "./res/shaders/game.fs");
    Uniforms program_uniforms;
    gl_get_all_uniform_locations(program, program_uniforms);

    glUseProgram(program);
    asset_image img = asset_image_load_rgb("./res/box texture.png");
    GLuint asset_texture = gl_texture_load(&img);

    bool running = true;
    SDL_Event evt;

    global_arena_debug();

    while (running) {
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

    return EXIT_SUCCESS;
}
#endif
