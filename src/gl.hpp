#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>

#include "./alloc_ctx.hpp"
#include "./assets.hpp"
#include "./os.hpp"
#include "./util.hpp"

void gl_init() {
    glClearColor(220.0 / 255.0, 220.0 / 255.0, 255.0 / 255.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    /* glEnable(GL_CULL_FACE); */
    /* glCullFace(GL_BACK); */
}

void gl_get_all_uniform_locations(GLuint program,
                                  std::unordered_map<std::string, GLint> &uniforms) {
    GLint num_uniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);

    GLchar name_data[256];
    for (int uniform = 0; uniform < num_uniforms; ++uniform) {
        GLint array_size = 0;
        GLenum type = 0;
        GLsizei actual_length = 0;
        glGetActiveUniform(program, uniform, 256, &actual_length, &array_size, &type,
                           name_data);
    }
}

struct gl_asset {
    GLuint EBO;
    GLuint VBO;
    GLuint VAO;
    u32 index_size;
};

void gl_asset_draw(gl_asset *asset) {
    glBindVertexArray(asset->VAO);
    glDrawElements(GL_TRIANGLES, asset->index_size, GL_UNSIGNED_SHORT, 0);
}

gl_asset gl_asset_load(asset_tinyobj asset) {
    GLuint EBO, VBO, VAO;

    float vertices[] = {// Positions         // Texture Coords
                        0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
                        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f};

    u16 indexes[] = {
        0, 1, 2, 2, 3, 0,
    };

    // Generate VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate VBO and bind it
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Generate VBO and bind it
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    // Define vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return {
        .EBO = EBO,
        .VBO = VBO,
        .VAO = VAO,
        .index_size = ARRAY_LEN(indexes),
    };
}

struct ui_rect {
    f32 x;
    f32 y;
    f32 w;
    f32 h;
};

gl_asset gl_ui_rect_load(ui_rect rect, ui_rect text_rect) {
    GLuint EBO, VBO, VAO;

    float vertices[] = {
        // Positions         // Texture Coords
        rect.x,
        rect.y,
        text_rect.x,
        text_rect.y,
        rect.x + rect.w,
        rect.y,
        text_rect.x + text_rect.w,
        text_rect.y,
        rect.x + rect.w,
        rect.y + rect.h,
        text_rect.x + text_rect.w,
        text_rect.y + text_rect.h,
        rect.x,
        rect.y + rect.h,
        text_rect.x,
        text_rect.y + text_rect.h,
    };

    u16 indexes[] = {
        0, 1, 2, 2, 3, 0,
    };

    // Generate VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    // Define vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return {
        .EBO = EBO,
        .VBO = VBO,
        .VAO = VAO,
        .index_size = ARRAY_LEN(indexes),
    };
}

GLuint gl_texture_load(asset_image *img) {
    assert(img->data);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->w, img->h, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 img->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

GLuint gl_shader_load(const char *vs, const char *fs) {
    const auto defer_lock = global_ctx_set_temporary();
    str vertex_source = read_entire_file(vs);
    str fragment_source = read_entire_file(fs);

    assert(vertex_source.s);
    assert(fragment_source.s);

    // Shader program handle
    GLuint program;

    // Load and compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source.s, NULL);
    glCompileShader(vertex_shader);

    // Load and compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
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

        char *log = (char *)global_ctx->alloc(max_length);

        glGetProgramInfoLog(program, max_length, &log_length, log);
        fprintf(stderr, "Shader error!!!: %s - %s\n", vs, fs);
        fprintf(stderr, "program log: %s\n", log);
        if (vertex_success == GL_FALSE) {
            glGetShaderInfoLog(vertex_shader, max_length, &log_length, log);
            fprintf(stderr, "vertex shader log: %s\n", log);
        }
        if (fragment_success == GL_FALSE) {
            glGetShaderInfoLog(fragment_shader, max_length, &log_length, log);
            fprintf(stderr, "fragment shader log: %s\n", log);
        }
        exit(1);
    }

    // Delete shaders since they're linked into program now and no longer
    // necessary
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}
