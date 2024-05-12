#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>

#include "src/alloc_ctx.hpp"
#include "src/assets.hpp"
#include "src/os.hpp"
#include "src/string.hpp"
#include "src/util.hpp"

static void gl_init() {
    glClearColor(220.0 / 255.0, 220.0 / 255.0, 255.0 / 255.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

struct gl_draw_object {
    GLuint EBO;
    GLuint VBO;
    GLuint VAO;
    u32 count;
};

static void gl_asset_draw(gl_draw_object *asset) {
    glBindVertexArray(asset->VAO);
    glDrawElements(GL_TRIANGLES, asset->count, GL_UNSIGNED_INT, 0);
}

static gl_draw_object gl_asset_load(string_view filename) {
    ctx_temp_lock();

    gl_draw_object mesh;
    fastObjMesh *objmesh = asset_fastobj_parse(filename);
    assert(objmesh);
    defer(asset_cleanup(objmesh));

    for (u32 i = 0; i < objmesh->face_count; i++) {
        assert(objmesh->face_vertices[i] == 3);
    }

    f32 *vertex_buffer =
        (f32 *)ctx->temp.alloc(objmesh->index_count * sizeof(f32) * 5);
    u32 *index_buffer =
        (u32 *)ctx->temp.alloc(objmesh->index_count * sizeof(u32));

    for (u32 i = 0; i < objmesh->index_count; i++) {
        fastObjIndex indexes = objmesh->indices[i];

        vertex_buffer[i * 5 + 0] = objmesh->positions[indexes.p * 3 + 0];
        vertex_buffer[i * 5 + 1] = objmesh->positions[indexes.p * 3 + 1];
        vertex_buffer[i * 5 + 2] = objmesh->positions[indexes.p * 3 + 2];
        vertex_buffer[i * 5 + 3] = objmesh->texcoords[indexes.t * 2 + 0];
        vertex_buffer[i * 5 + 4] = objmesh->texcoords[indexes.t * 2 + 1];
    }
    for (u32 i = 0; i < objmesh->index_count; i++) {
        index_buffer[i] = i;
    }

    mesh.count = objmesh->index_count;

    glGenVertexArrays(1, &mesh.VAO);
    glBindVertexArray(mesh.VAO);

    glGenBuffers(1, &mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 5 * objmesh->index_count, vertex_buffer,
                 GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, objmesh->index_count * sizeof(u32),
                 index_buffer, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32),
                          (void *)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    return mesh;
}

struct ui_rect {
    f32 x;
    f32 y;
    f32 w;
    f32 h;
};

static gl_draw_object gl_ui_rect_load(ui_rect rect, ui_rect text_rect) {
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

    u32 indexes[] = {
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return {
        .EBO = EBO,
        .VBO = VBO,
        .VAO = VAO,
        .count = ARRAY_LEN(indexes),
    };
}

static GLuint gl_texture_load(asset_image *img) {
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

typedef std::unordered_map<string_view, GLint> Uniforms;

struct gl_program {
    GLuint program;
    Uniforms uniforms;
};

static gl_program gl_shader_load(string_view vs_filename, string_view fs_filename) {
    // Shader program handle
    GLuint program;
    Uniforms uniforms;
    string_view vs = asset_downloaded_file_get(vs_filename);
    string_view fs = asset_downloaded_file_get(fs_filename);

    // Load and compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs.start, NULL);
    glCompileShader(vertex_shader);

    // Load and compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs.start, NULL);
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

        char *log = (char *)ctx->temp.alloc(max_length);

        glGetProgramInfoLog(program, max_length, &log_length, log);
        fprintf(stderr, "Shader error!!!:\n");
        fprintf(stderr, "program log: %s\n", log);
        if (vertex_success == GL_FALSE) {
            glGetShaderInfoLog(vertex_shader, max_length, &log_length, log);
            fprintf(stderr, "vertex shader: %s\nlog: %s\n", vs.begin(), log);
        }
        if (fragment_success == GL_FALSE) {
            glGetShaderInfoLog(fragment_shader, max_length, &log_length, log);
            fprintf(stderr, "fragment shader %s\nlog: %s\n", fs.begin(), log);
        }
        exit(1);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    GLint num_uniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);

    GLchar name_data[256];
    for (int uniform = 0; uniform < num_uniforms; uniform++) {
        GLint array_size = 0;
        GLenum type = 0;
        GLsizei actual_length = 0;
        glGetActiveUniform(program, uniform, 256, &actual_length, &array_size, &type,
                           name_data);
        GLint location = glGetUniformLocation(program, name_data);
        string name_str = string_make(name_data, actual_length);
        uniforms[name_str] = location;
    }

    return {program, uniforms};
}

static void gl_shader_destory(gl_program *p) {
    glDeleteShader(p->program);

    // TODO(mike): fix
#if 0
    for (auto &it : p->uniforms) {
        /* string non_const = it.first; */
        /* string_destroy(&non_const); */
    }
#endif
    p->uniforms.clear();
}
