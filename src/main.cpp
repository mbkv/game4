#include "./assets.hpp"
#include "./gl.hpp"
#include "./util.hpp"
#include "./vendors.hpp"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <stdio.h>
#include <stdlib.h>

SDL_Window* win;
SDL_GLContext context;

static bool make_window(glm::vec2 window_size)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
		return false;
	}

	win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, window_size[0],
		window_size[1],
		SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
	if (win == nullptr) {
		fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
		return false;
	}

	context = SDL_GL_CreateContext(win);
	if (context == nullptr) {
		fprintf(stderr, "SDL_GL_CreateContext Error: %s\n", SDL_GetError());
		return false;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	glewExperimental = true;
	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		fprintf(stderr, "glewInit Error: %s\n", glewGetErrorString(glew_err));
		return false;
	}

	return true;
}

int main()
{
	defer(temp_alloc_debug());

	asset_tinyobj asset = asset_tinyobj_parse("./res/box.obj");

	defer(asset_cleanup(asset));

	glm::vec2 window_size { 1280, 720 };
	if (!make_window(window_size)) {
		return EXIT_FAILURE;
	}

	defer(SDL_Quit());

	gl_init();
	GLuint program = gl_shader_load("./res/shaders/game.vs", "./res/shaders/game.fs", &temp_allocator);

	glUseProgram(program);

	bool running = true;
	SDL_Event evt;

	while (running) {
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				running = false;
				break;
			}
		}

		temp_alloc_freeall();
	}

	return EXIT_SUCCESS;
}
