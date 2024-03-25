#include "./alloc_ctx.hpp"
#include "./assets.hpp"
#include "./gl.hpp"
#include "./util.hpp"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <stdio.h>
#include <stdlib.h>

SDL_Window *win;
SDL_GLContext context;

static bool make_window(glm::vec2 window_size) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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
    defer(temp_alloc_debug());

    glm::vec2 window_size{1920, 1080};
    if (!make_window(window_size)) {
        return EXIT_FAILURE;
    }

    defer(SDL_Quit());

    gl_init();

    glm::mat4 camera =
        glm::lookAt(glm::vec3{3, 0, 3}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});

    glm::mat4 position = glm::identity<glm::mat4>();

    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

    /* asset_tinyobj asset_box = asset_tinyobj_parse("./res/box textured.obj"); */
    /* gl_asset x = gl_asset_load(asset_box); */
    gl_draw_object asset_box = gl_asset_load("./res/box textured.obj");

    /* /1* defer(asset_cleanup(asset_box)); *1/ */

    GLuint program = gl_shader_load("./res/shaders/game.vs", "./res/shaders/game.fs");
    Uniforms program_uniforms;
    gl_get_all_uniform_locations(program, program_uniforms);
    glUseProgram(program);

    /* GLuint ui_program = gl_shader_load("./res/shaders/ui.vs",
     * "./res/shaders/ui.fs");
     */

    /* /1* GLint ui_program_screen_size = glGetUniformLocation(ui_program,
     * "uScreenSize"); *1/ */
    /* Uniforms ui_program_uniforms; */
    /* gl_get_all_uniform_locations(ui_program, ui_program_uniforms); */

    /* glUseProgram(ui_program); */
    /* glUniform2f(ui_program_uniforms["uScreenSize"], window_size[0],
     * window_size[1]);
     */

    asset_image img = asset_image_load_rgb("./res/box texture.png");
    GLuint asset_texture = gl_texture_load(&img);
    gl_draw_object asset_texture_box = gl_ui_rect_load(
        {100.0, 100.0, ((f32)img.w * 3), ((f32)img.h * 3)}, {0.0, 0.0, 1.0, 1.0});

    bool running = true;
    SDL_Event evt;

    temp_alloc_debug();

    while (running) {
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    running = false;
                if (evt.key.keysym.scancode == SDL_SCANCODE_F4) {
                    GLint polygonMode;
                    glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
                    if (polygonMode == GL_LINE) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    } else {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    }
                }
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

        temp_alloc_freeall();
        SDL_GL_SwapWindow(win);
    }

    return EXIT_SUCCESS;
}
