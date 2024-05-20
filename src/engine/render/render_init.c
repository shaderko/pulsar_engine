/**
 * @file render_init.c
 * @author https://github.com/shaderko
 * @brief Initialization functions for render.
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <glad/glad.h>
#include "render_init.h"
#include "../util/util.h"
#include "../io/io.h"

void render_init_shaders(WindowRender *render)
{
    render->shader = render_shader_create_comp("./shaders/voxel");
    render->shader_screen = render_shader_create_frag_vert("./shaders/screen");

    // By default use the comp shader
    glUseProgram(render->shader);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
}

u32 render_shader_create_frag_vert(const char *path)
{
    // Create path
    size_t len = strlen(path);

    char *vertex_shader_path = malloc(len + 6);
    memset(vertex_shader_path, 0, len + 6);
    strcat(vertex_shader_path, path);
    strcat(vertex_shader_path, ".vert");

    char *fragment_shader_path = malloc(len + 6);
    memset(fragment_shader_path, 0, len + 6);
    strcat(fragment_shader_path, path);
    strcat(fragment_shader_path, ".frag");

    // Create shaders
    int success;
    char log[512];

    File file_vertex = io_file_read(vertex_shader_path);
    if (!file_vertex.is_valid)
    {
        ERROR_EXIT("error reading shader: %s\n", vertex_shader_path);
    }

    u32 shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vertex, 1, (const char *const *)&file_vertex, NULL);
    glCompileShader(shader_vertex);
    glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_vertex, 512, NULL, log);
        ERROR_EXIT("error compiling vertex shader. %s\n", log);
    }

    File file_fragment = io_file_read(fragment_shader_path);
    if (!file_fragment.is_valid)
    {
        ERROR_EXIT("error reading shader: %s\n", fragment_shader_path);
    }

    u32 shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fragment, 1, (const char *const *)&file_fragment, NULL);
    glCompileShader(shader_fragment);
    glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_fragment, 512, NULL, log);
        ERROR_EXIT("error compiling fragment shader. %s\n", log);
    }

    u32 shader = glCreateProgram();
    glAttachShader(shader, shader_vertex);
    glAttachShader(shader, shader_fragment);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader, 512, NULL, log);
        ERROR_EXIT("error linking shader. %s\n", log);
    }

    free(file_vertex.data);
    free(file_fragment.data);

    free(vertex_shader_path);
    free(fragment_shader_path);

    return shader;
}

u32 render_shader_create_comp(const char *path)
{
    // Create path
    size_t len = strlen(path);

    char *compute_shader_path = malloc(len + 6);
    memset(compute_shader_path, 0, len + 6);
    strcat(compute_shader_path, path);
    strcat(compute_shader_path, ".comp");

    // Create the shader
    int success;
    char log[512];

    File file_compute = io_file_read(compute_shader_path);
    if (!file_compute.is_valid)
    {
        ERROR_EXIT("error reading shader: %s\n", compute_shader_path);
    }

    u32 shader_compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader_compute, 1, (const char *const *)&file_compute, NULL);
    glCompileShader(shader_compute);
    glGetShaderiv(shader_compute, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_compute, 512, NULL, log);
        ERROR_EXIT("error compiling compute shader. %s\n", log);
    }

    u32 shader = glCreateProgram();
    glAttachShader(shader, shader_compute);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader, 512, NULL, log);
        ERROR_EXIT("error linking shader. %s\n", log);
    }

    free(file_compute.data);

    free(compute_shader_path);

    return shader;
}
