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
#include <stdlib.h>
#include <time.h>

#include "render.h"
#include "../util/util.h"
#include "../window/window.h"
#include "../camera/camera.h"
#include "../object/map/scene.h"
#include "gpu_cache/gpu_cache_manager.h"

#define NUM_RAYS_X 100
#define NUM_RAYS_Y 100

static WindowRender *active_render = NULL;
static GLuint heightMapArray = 0;

static WindowRender *Init()
{
    WindowRender *render = malloc(sizeof(WindowRender));
    if (!render)
        ERROR_EXIT("[ERROR] Failed to allocate memory for Window Render.");

    // Set up shaders
    render_init_shaders(render);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // Set this render as active
    active_render = render;

    AWindowRender->RenderScreenInit();

    AGpuCache.Init();

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

static void RenderChunks(Scene *scene, Camera *camera)
{
    // puts("[INFO] Preparing chunks for rendering");
    static gpu_cache_manager_t *gpu_manager = NULL;
    if (!gpu_manager)
        gpu_manager = AGpuCache.Get();

    // Shader program using vertex / fragment shaders
    glUseProgram(active_render->render_shader);

    // Start timer to track how much time it takes to prepare all data for the gpu
    GLuint queries[2];
    glGenQueries(2, queries);
    glBeginQuery(GL_TIME_ELAPSED, queries[0]);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(active_render->render_shader, "projection"), 1, GL_FALSE, &camera->projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(active_render->render_shader, "view"), 1, GL_FALSE, &camera->view[0][0]);
    glUniform3fv(glGetUniformLocation(active_render->render_shader, "viewPos"), 1, camera->position);

    // We can only create this once since the chunks won't change only their height map
    static bool allocated = false; // This will change in the future if we want to add a chunk mid way through, but if the chunk is change
    // it doesn't matter since we will change the height map
    static GLuint vao, vbo, ebo;

    if (!allocated)
    {
        puts("[INFO] Allocating GPU buffers");
        allocated = true;

        float vertices[] = {
            // positions        // texture coords
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 0
            1.0f, 0.0f, 1.0f, 1.0f, 0.0f, // 1
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // 2
            0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 3
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 4
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 5
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // 6
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // 7
        };

        // Define the indices for the cube with correct winding order (CCW)
        unsigned int indices[] = {
            // Front face
            0, 1, 2,
            2, 3, 0,
            // Back face
            6, 5, 4,
            4, 7, 6,
            // Left face
            4, 0, 3,
            3, 7, 4,
            // Right face
            1, 5, 6,
            6, 2, 1,
            // Top face
            3, 2, 6,
            6, 7, 3,
            // Bottom face
            4, 5, 1,
            1, 0, 4};

        // Create VAO, VBO, and EBO
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // Texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, gpu_manager->gpu_chunks_buffer);

        // Instance attribute: position x
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GPUChunk), offsetof(GPUChunk, x));
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1); // Update per instance

        // Instance attribute: texture index
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GPUChunk), offsetof(GPUChunk, textureIndex));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1); // Update per instance

        glBindVertexArray(0);
    }

    glBindVertexArray(vao);

    // Bind the height map texture array
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D_ARRAY, gpu_manager->gpu_textures_storage_buffer);
    glUniform1i(glGetUniformLocation(active_render->render_shader, "heightMaps"), 10);

    // Bind the VAO and draw instanced
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, scene->chunks_size);
    glBindVertexArray(0);

    // End the timer
    glEndQuery(GL_TIME_ELAPSED);
    GLuint64 timeElapsed;
    glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &timeElapsed);
    printf("[INFO] Time taken for rendering: %f ms\n", timeElapsed / 1000000.0);
}

static void RayMarchChunkHeightTexture(Scene *scene, Camera *camera)
{
    static int *randomPositions = NULL;
    if (!randomPositions)
        randomPositions = malloc(sizeof(int) * (NUM_RAYS_X * NUM_RAYS_Y) * 2);

    // Generate random coordinates
    int screenWidth = 1920;  // Example screen width
    int screenHeight = 1080; // Example screen height

    for (int i = 0; i < (NUM_RAYS_X * NUM_RAYS_Y) * 2; i += 2)
    {
        randomPositions[i] = rand() % screenWidth;
        randomPositions[i + 1] = rand() % screenHeight;
    }

    static GLuint randomPositionsSSBO;
    if (!randomPositionsSSBO)
    {
        glGenBuffers(1, &randomPositionsSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, randomPositionsSSBO);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, randomPositionsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * (NUM_RAYS_X * NUM_RAYS_Y) * 2, randomPositions, GL_STATIC_DRAW);

    static GLuint resultBuffer;
    if (!resultBuffer)
    {
        // Create the result buffer
        glGenBuffers(1, &resultBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, resultBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec3) * NUM_RAYS_X * NUM_RAYS_Y, 0, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, resultBuffer);
    }

    glUseProgram(active_render->ray_march_compute_shader);

    static gpu_cache_manager_t *gpu_manager = NULL;
    if (!gpu_manager)
        gpu_manager = AGpuCache.Get();

    // Start timer to track how much time it takes to prepare all data for the gpu
    GLuint queries[2];
    glGenQueries(2, queries);
    glBeginQuery(GL_TIME_ELAPSED, queries[0]);

    // Set uniforms
    glUniform3fv(glGetUniformLocation(active_render->ray_march_compute_shader, "cameraPos"), 1, camera->position);
    glUniformMatrix4fv(glGetUniformLocation(active_render->ray_march_compute_shader, "projection"), 1, GL_FALSE, &camera->projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(active_render->ray_march_compute_shader, "view"), 1, GL_FALSE, &camera->view[0][0]);
    glUniform1i(glGetUniformLocation(active_render->ray_march_compute_shader, "numChunks"), gpu_manager->chunks_buffer_index);

    AGpuCache.Prepare();

    // End the data preparation timer
    glEndQuery(GL_TIME_ELAPSED);
    GLuint64 timeElapsed;
    glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &timeElapsed);
    // printf("[INFO] Time taken for data preparation: %f ms\n", timeElapsed / 1000000.0);

    // Prepare timer for the actual ray marching
    glGenQueries(2, queries);
    glBeginQuery(GL_TIME_ELAPSED, queries[0]);

    // Dispatch compute shader
    glDispatchCompute(NUM_RAYS_X, NUM_RAYS_Y, 1);

    // Synchronize compute shader
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Fetch the next chunks position
    vec3 nextChunkPosition[NUM_RAYS_X * NUM_RAYS_Y];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, resultBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec3) * NUM_RAYS_X * NUM_RAYS_Y, &nextChunkPosition);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // if (chunk)
    // {
    //     // Adjust the height map texture array with the new data (eg. copy the data from the passed textureat 0 to height map array at index of chunk->gpu_chunk->textureIndex)
    //     // glBindTexture(GL_TEXTURE_2D, chunk->heightMap);

    //     // // Read the updated texture data
    //     // GLubyte *data = (GLubyte *)malloc(256 * 256 * sizeof(GLubyte));
    //     // glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, data);

    //     // // Bind the texture array and update the specific layer
    //     // glBindTexture(GL_TEXTURE_2D_ARRAY, heightMapArray);
    //     // glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, chunk->gpu_chunk->textureIndex, 256, 256, 1, GL_RED, GL_UNSIGNED_BYTE, data);

    //     // Free the temporary data buffer
    //     // free(data);

    //     // Unbind the textures
    //     // glBindTexture(GL_TEXTURE_2D, 0);
    //     // glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    //     // Clean up
    //     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //     free(serialized_chunk.data);

    //     chunk = NULL;
    // }

    // Get the next chunk at nextChunkPosition
    for (size_t i = 0; i < NUM_RAYS_X * NUM_RAYS_Y; i++)
    {
        vec3 *position = nextChunkPosition[i];

        Chunk *chunk = AScene.GetChunkAt(scene, position);
        AGpuCache.AddToCache(chunk);
    }

    // End the ray marching timer
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &timeElapsed);
    printf("[INFO] Time taken for ray marching: %f ms\n", timeElapsed / 1000000.0);
}

static void RenderBegin(Window *window, Camera *camera)
{
    SDL_GL_MakeCurrent(window->sdl_window, window->context);
    active_render = window->render;
    glUseProgram(window->render->render_shader);

    glEnable(GL_DEPTH_TEST);

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
    // SDL_GL_MakeCurrent(window->sdl_window, window->context);
    // active_render = window->render;
    // glUseProgram(window->render->shader);

    // GLfloat lightPosition[] = {position[0], position[1], position[2]}; // x, y, z
    // GLfloat lightAmbient[] = {0, 0.2f, 0.2f};                          // r, g, b
    // GLfloat lightDiffuse[] = {0, 1, 1};                                // r, g, b
    // GLfloat lightSpecular[] = {.2f, .2f, .2f};                         // r, g, b

    // glUniform3fv(glGetUniformLocation(window->render->shader, "light.position"), 1, lightPosition);
    // glUniform3fv(glGetUniformLocation(window->render->shader, "light.ambient"), 1, lightAmbient);
    // glUniform3fv(glGetUniformLocation(window->render->shader, "light.diffuse"), 1, lightDiffuse);
    // glUniform3fv(glGetUniformLocation(window->render->shader, "light.specular"), 1, lightSpecular);
    // glUniform3fv(glGetUniformLocation(window->render->shader, "light.constant"), 1, &(float){1.0f});
    // glUniform3fv(glGetUniformLocation(window->render->shader, "light.linear"), 1, &(float){0.09f});
    // glUniform3fv(glGetUniformLocation(window->render->shader, "light.quadratic"), 1, &(float){0.032f});

    // GLfloat ambient[] = {0, .3f, .3f};       // x, y, z
    // GLfloat diffuse[] = {0, 0, 1};           // r, g, b
    // GLfloat specular[] = {0.5f, 0.5f, 0.5f}; // r, g, b
    // GLfloat shininess = 0;

    // glUniform3fv(glGetUniformLocation(window->render->shader, "material.ambient"), 1, ambient);
    // glUniform3fv(glGetUniformLocation(window->render->shader, "material.diffuse"), 1, diffuse);
    // glUniform3fv(glGetUniformLocation(window->render->shader, "material.specular"), 1, specular);
    // glUniform3fv(glGetUniformLocation(window->render->shader, "material.shininess"), 1, &shininess);
}

static void Destroy(WindowRender *render)
{
    // Free shaders
    glDeleteProgram(render->render_shader);
    glDeleteProgram(render->ray_march_compute_shader);
    glDeleteProgram(render->shader_screen);

    // Free buffers
    glDeleteBuffers(1, &render->vbo);
    glDeleteBuffers(1, &render->ebo);
    glDeleteVertexArrays(1, &render->vao);

    free(render);

    puts("Render destroyed");

    return;
}

struct AWindowRender AWindowRender[1] = {{Init, Destroy, RenderScreenInit, RenderScreen, RenderChunks, RayMarchChunkHeightTexture, RenderBegin, RenderEnd, RenderLight}};