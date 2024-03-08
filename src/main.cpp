#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "./util.hpp"
#include "./vendors.hpp"
#include "./assets.hpp"

SDL_Window* win;
SDL_GLContext context;

static bool make_window(glm::vec2 window_size) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
		return false;
	}

	win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, window_size[0],
			window_size[1],
			SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS |
			SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL
			);
	if (win == nullptr) {
		fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
		return false;
	}

	context = SDL_GL_CreateContext(win);
	if (context == nullptr) {
		fprintf(stderr, "SDL_GL_CreateContext Error: %s\n", SDL_GetError());
		return false;
	}
	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		fprintf(stderr, "glewInit Error: %s\n", glewGetErrorString(glew_err));
		return false;
	}

	return true;
}

void gl_init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void
tinyobj_file_reader_impl(void * ctx, const char * filename, int is_mtl, const char * obj_filename, char **buf, size_t *len)
{
	auto file = read_entire_file(filename, temp_alloc);
	*buf = file.data;
	*len = file.length;
	
}


int main()
{
	string_t box = read_entire_file((char *)"./res/box.obj", temp_alloc);
	defer(temp_alloc_freeall());
	defer(temp_alloc_debug());
	tinyobj_attrib_t attrib;
	tinyobj_shape_t *shapes;
	size_t num_shapes;
	tinyobj_material_t *materials;
	size_t num_materials;


	tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, 
			"res/box.obj",
			tinyobj_file_reader_impl, nullptr, 0);
	defer({
		tinyobj_attrib_free(&attrib);
		tinyobj_shapes_free(shapes, num_shapes);
		tinyobj_materials_free(materials, num_materials);
	});
	printf("%s", box.data);
	

	/* glm::vec2 window_size{1280, 720}; */
	/* if (!make_window(window_size)) { */
	/* 	return EXIT_FAILURE; */
	/* } */

	/* gl_init(); */

	/* defer(SDL_Quit()); */

	/* bool running = true; */
	/* SDL_Event evt; */



	/* while (running) { */
	/* 	while (SDL_PollEvent(&evt)) { */
	/* 		switch (evt.type) { */
	/* 			case SDL_QUIT: */
	/* 				running = false; */
	/* 				break; */
	/* 		} */


	/* 	} */


		
		
	/* } */



	return EXIT_SUCCESS;
}
