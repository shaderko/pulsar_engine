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
#include <time.h>

#include "render.h"
#include "../util/util.h"
#include "../window/window.h"
#include "../camera/camera.h"
#include "../object/map/scene.h"

static WindowRender *active_render = NULL;

static WindowRender *Init()
{
    WindowRender *render = malloc(sizeof(WindowRender));
    if (!render)
        ERROR_EXIT("[ERROR] Failed to allocate memory for Window Render.");

    // Set up shaders
    render_init_shaders(render);

    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_DEPTH_CLAMP);

    // Enable face culling
    // glEnable(GL_CULL_FACE);
    // // Cull back faces
    // glCullFace(GL_BACK);
    // // Define front faces (counter-clockwise winding)
    // glFrontFace(GL_CCW);

    // Set this render as active
    active_render = render;

    AWindowRender->RenderScreenInit();

    return render;
}

static void SpriteRender();

static void RenderScreenInit()
{
    float vertices[] = {
        // Positions   // TexCoords
        -1.0f, 1.0f, 0.0f, 0.0f,  // Top-left
        -1.0f, -1.0f, 0.0f, 1.0f, // Bottom-left
        1.0f, -1.0f, 1.0f, 1.0f,  // Bottom-right

        -1.0f, 1.0f, 0.0f, 0.0f, // Top-left
        1.0f, -1.0f, 1.0f, 1.0f, // Bottom-right
        1.0f, 1.0f, 1.0f, 0.0f   // Top-right
    };

    glGenVertexArrays(1, &active_render->screen_vao);
    glGenBuffers(1, &active_render->screen_vbo);

    glBindVertexArray(active_render->screen_vao);
    glBindBuffer(GL_ARRAY_BUFFER, active_render->screen_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO
}

static void RenderScreen(Camera *camera)
{
    glUseProgram(active_render->shader_screen);

    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, camera->image_out); // Bind the texture
    glBindVertexArray(active_render->screen_vao);    // Bind VAO
    glDrawArrays(GL_TRIANGLES, 0, 6);                // Draw the quad
}

static void RenderSceneChunks(Scene *scene, Camera *camera, int width, int height)
{
    glUseProgram(active_render->shader);

    // Start timer to track how much time it takes to render
    GLuint queries[2];
    glGenQueries(2, queries);
    glBeginQuery(GL_TIME_ELAPSED, queries[0]);

    static SerializedScene serialized_scene = {0};
    if (serialized_scene.chunks_data_size == 0)
    {
        puts("[INFO] Serializing scene.");
        serialized_scene = AScene.SerializeChunks(scene);

        // Now we pass the data to the gpu

        // puts("Dispatching compute shader");
        // puts("[INFO] Creating the gpu chunk buffer");
        // printf("%u\n", serialized_scene.chunks_data_size);

        // Create and initialize buffer object for chunk data
        GLuint gpu_chunk_buffer;
        glGenBuffers(1, &gpu_chunk_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_chunk_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, 1728 * sizeof(GPUChunk), serialized_scene.gpu_chunks, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gpu_chunk_buffer);

        // puts("[INFO] Creating the chunk buffer");

        GLuint chunk_buffer;
        glGenBuffers(1, &chunk_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunk_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, serialized_scene.chunks_data_size * sizeof(unsigned int), serialized_scene.chunks_data, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, chunk_buffer);
    }

    glUniform3fv(glGetUniformLocation(active_render->shader, "cameraPos"), 1, camera->position);
    glUniformMatrix4fv(glGetUniformLocation(active_render->shader, "projection"), 1, GL_FALSE, &camera->projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(active_render->shader, "view"), 1, GL_FALSE, &camera->view[0][0]);

    glEndQuery(GL_TIME_ELAPSED);
    GLuint64 timeElapsed;
    glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &timeElapsed);
    printf("Time taken before render: %f ms\n", timeElapsed / 1000000.0);

    glGenQueries(2, queries);
    glBeginQuery(GL_TIME_ELAPSED, queries[0]);

    // Dispatch compute shader with appropriate work group count
    glDispatchCompute(width / 48, height / 32, 1);

    // Synchronize compute shader
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // puts("Chunk data computed");

    // Clean up
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // glDeleteBuffers(1, &gpu_chunk_buffer);
    // glDeleteBuffers(1, &chunk_buffer);

    // free(serialized_scene.chunks_data);
    // free(serialized_scene.gpu_chunks);

    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &timeElapsed);
    printf("Time taken: %f ms\n", timeElapsed / 1000000.0);
}

static void RenderBegin(Window *window, Camera *camera)
{
    SDL_GL_MakeCurrent(window->sdl_window, window->context);
    active_render = window->render;
    glUseProgram(window->render->shader);

    glEnable(GL_DEPTH_TEST);

    // glUniformMatrix4fv(glGetUniformLocation(window->render->shader, "projection"), 1, GL_FALSE, &camera->projection[0][0]);
    // glUniformMatrix4fv(glGetUniformLocation(window->render->shader, "view"), 1, GL_FALSE, &camera->view[0][0]);

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

struct AWindowRender AWindowRender[1] = {{Init, Destroy, RenderScreenInit, RenderScreen, RenderSceneChunks, RenderBegin, RenderEnd, RenderLight}};