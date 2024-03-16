/**
 * @file render.c
 * @author https://github.com/shaderko
 * @brief Render API
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <glad/glad.h>
#include "render.h"
#include "../window/window.h"
#include "../util/util.h"

static WindowRender *active_render = NULL;

static WindowRender *Init()
{
    WindowRender *render = malloc(sizeof(WindowRender));
    if (!render)
    {
        ERROR_EXIT("Failed to allocate render struct\n");
    }

    // Set up shaders
    render->shader = render_shader_create_name("./shaders/default");

    glUseProgram(render->shader);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);

    AWindowRender->RenderInitMesh(render);

    // Set this render as active
    active_render = render;

    return render;
}

static void RenderInitMesh(WindowRender *render)
{
    glGenVertexArrays(1, &render->vao);
    glGenBuffers(1, &render->vbo);
    glGenBuffers(1, &render->ebo);

    glBindVertexArray(render->vao);
    glBindBuffer(GL_ARRAY_BUFFER, render->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->ebo);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // UV attribute
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)(3 * sizeof(f32)));
    // glEnableVertexAttribArray(1);
}

static void RenderMesh(Model *model, vec3 position, vec3 scale)
{
    glBindBuffer(GL_ARRAY_BUFFER, active_render->vbo);
    glBufferData(GL_ARRAY_BUFFER, model->verticies_count * sizeof(vec3), model->verticies, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, active_render->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indicies_count * sizeof(unsigned int), model->indicies, GL_STATIC_DRAW);

    mat4x4 model_matrix = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}};

    mat4x4_translate(model_matrix, position[0], position[1], position[2]);
    mat4x4_scale_aniso(model_matrix, model_matrix, scale[0] / 2, scale[1] / 2, scale[2] / 2);

    glUniformMatrix4fv(glGetUniformLocation(active_render->shader, "model"), 1, GL_FALSE, &model_matrix[0][0]);

    glBindVertexArray(active_render->vao);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, model->indicies_count, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}

static void RenderBegin(Window *window)
{
    SDL_GL_MakeCurrent(window->sdl_window, window->context);
    active_render = window->render;
    glUseProgram(window->render->shader);

    float aspect_ratio = (float)window->width / (float)window->height;
    mat4x4_ortho(window->camera->projection, -window->camera->distance * aspect_ratio, window->camera->distance * aspect_ratio, -window->camera->distance, window->camera->distance, 1.0f, 100000.0f);
    glEnable(GL_DEPTH_TEST);
    glUniformMatrix4fv(glGetUniformLocation(window->render->shader, "projection"), 1, GL_FALSE, &window->camera->projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(window->render->shader, "view"), 1, GL_FALSE, &window->camera->view[0][0]);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void RenderEnd(Window *window)
{
    SDL_GL_MakeCurrent(window->sdl_window, window->context);
    SDL_GL_SwapWindow(window->sdl_window);
}

static void Destroy(WindowRender *render)
{
    // Free shaders
    glDeleteProgram(render->shader);

    // Free buffers
    glDeleteBuffers(1, &render->vbo);
    glDeleteBuffers(1, &render->ebo);
    glDeleteVertexArrays(1, &render->vao);

    free(render);

    puts("Render destroyed");

    return;
}

struct AWindowRender AWindowRender[1] = {{Init, Destroy, RenderInitMesh, RenderMesh, RenderBegin, RenderEnd}};