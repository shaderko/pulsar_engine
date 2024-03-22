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

    // // Enable face culling
    // glEnable(GL_CULL_FACE);
    // // Cull back faces
    // glCullFace(GL_BACK);
    // // Define front faces (counter-clockwise winding)
    // glFrontFace(GL_CCW);

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

static void RenderMesh(Model *model, mat4x4 transform)
{
    glBindVertexArray(model->vao);
    GLuint matrixVBO;
    glGenBuffers(1, &matrixVBO);
    glBindBuffer(GL_ARRAY_BUFFER, matrixVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4x4), &transform[0][0], GL_STATIC_DRAW);

    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4x4), (void *)(sizeof(float) * 4 * i));
        glVertexAttribDivisor(3 + i, 1); // Set to 0 for non-instanced drawing; typically would be 1 for instanced
    }

    // glUniformMatrix4fv(glGetUniformLocation(active_render->shader, "model"), 1, GL_FALSE, &model_matrix[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, model->indicies_count, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
    glDeleteBuffers(1, &matrixVBO);
}

static void BatchRenderMesh(Model *model, uint32_t vbo, size_t instanceCount)
{
    glBindVertexArray(model->vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Set up vertex attributes for the instance matrix
    for (GLuint i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(3 + i); // Assuming attributes 0, 1, 2 are used for position, normal, texture, etc.
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(mat4x4), (void *)(sizeof(vec4) * i));
        glVertexAttribDivisor(3 + i, 1); // This attribute only updates per-instance, not per-vertex
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, model->indicies_count, GL_UNSIGNED_INT, 0, instanceCount);

    glBindVertexArray(0);
}

static void RenderBegin(Window *window)
{
    SDL_GL_MakeCurrent(window->sdl_window, window->context);
    active_render = window->render;
    glUseProgram(window->render->shader);

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

static void RenderLight(Window *window, vec3 position)
{
    SDL_GL_MakeCurrent(window->sdl_window, window->context);
    active_render = window->render;
    glUseProgram(window->render->shader);

    GLfloat lightPosition[] = {position[0], position[1], position[2]}; // x, y, z
    GLfloat lightAmbient[] = {0, 0.2f, 0.2f};                          // r, g, b
    GLfloat lightDiffuse[] = {0, 1, 1};                                // r, g, b
    GLfloat lightSpecular[] = {.2f, .2f, .2f};                         // r, g, b

    glUniform3fv(glGetUniformLocation(window->render->shader, "light.position"), 1, lightPosition);
    glUniform3fv(glGetUniformLocation(window->render->shader, "light.ambient"), 1, lightAmbient);
    glUniform3fv(glGetUniformLocation(window->render->shader, "light.diffuse"), 1, lightDiffuse);
    glUniform3fv(glGetUniformLocation(window->render->shader, "light.specular"), 1, lightSpecular);
    glUniform3fv(glGetUniformLocation(window->render->shader, "light.constant"), 1, &(float){1.0f});
    glUniform3fv(glGetUniformLocation(window->render->shader, "light.linear"), 1, &(float){0.09f});
    glUniform3fv(glGetUniformLocation(window->render->shader, "light.quadratic"), 1, &(float){0.032f});

    GLfloat ambient[] = {0, .3f, .3f};       // x, y, z
    GLfloat diffuse[] = {0, 0, 1};           // r, g, b
    GLfloat specular[] = {0.5f, 0.5f, 0.5f}; // r, g, b
    GLfloat shininess = 0;

    glUniform3fv(glGetUniformLocation(window->render->shader, "material.ambient"), 1, ambient);
    glUniform3fv(glGetUniformLocation(window->render->shader, "material.diffuse"), 1, diffuse);
    glUniform3fv(glGetUniformLocation(window->render->shader, "material.specular"), 1, specular);
    glUniform3fv(glGetUniformLocation(window->render->shader, "material.shininess"), 1, &shininess);
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

struct AWindowRender AWindowRender[1] = {{Init, Destroy, RenderInitMesh, RenderMesh, BatchRenderMesh, RenderBegin, RenderEnd, RenderLight}};