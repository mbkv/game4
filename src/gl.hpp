#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "./os.hpp"
#include "./assets.hpp"
#include "./util.hpp"


void gl_init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

struct gl_asset
{
	GLuint VBO;
	GLuint VAO;
};

gl_asset
gl_asset_load(asset_tinyobj asset)
{
	// VBO and VAO handles
	GLuint VBO, VAO;

	// Vertex data 
	float vertices[] = {
		// Positions         // Texture Coords
		0.5f,  0.5f, 0.0f,   1.0f, 1.0f,  
		0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f
	};

	// Generate VAO and bind it
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Generate VBO and bind it
	glGenBuffers(1, &VBO); 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Define vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); 
	glEnableVertexAttribArray(1);

	return { VBO, VAO };
}

GLuint
gl_shader_load(const char *vs, const char *fs, allocator_t *allocator)
{
	str vertex_source = read_entire_file(vs, allocator);
	str fragment_source = read_entire_file(fs, allocator);

	// Shader program handle
	GLuint program;

	// Load and compile vertex shader
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source.s, NULL);
	glCompileShader(vertex_shader);

	// Load and compile fragment shader  
	GLuint  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source.s, NULL); 
	glCompileShader(fragment_shader);

	// Link shaders and check for errors
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);  
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE) {
		GLint vertex_success;
		GLint fragment_success;
		GLint max_length;
		GLint log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		max_length = log_length;

		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_success);
		if (vertex_success == GL_FALSE) {
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
			max_length = max(max_length, log_length);
		}

		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_success);
		if (fragment_success == GL_FALSE) {
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
			max_length = max(max_length, log_length);
		}
		
		char *log = (char * )allocator->alloc(max_length);
		defer(allocator->free(log));

		glGetProgramInfoLog(program, max_length, &log_length, log);
		fprintf(stderr, "Shader error!!!\n");
		fprintf(stderr, "program log: %s\n", log);
		if (vertex_success == GL_FALSE) {
			glGetShaderInfoLog(vertex_shader, max_length, &log_length, log);
			fprintf(stderr, "\nvertex shader log: %s\n\n", log);
		}
		if (fragment_success == GL_FALSE) {
			glGetShaderInfoLog(fragment_shader, max_length, &log_length, log);
			fprintf(stderr, "\nfragment shader log: %s\n\n", log);
		}
		exit(1);
	}

	// Delete shaders since they're linked into program now and no longer necessary
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return program;
}
