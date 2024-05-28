/**
 * @file render.h
 * @author https://github.com/shaderko
 * @brief Render API. We want to allow 3D and 2D rendering.
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef RENDER_H
#define RENDER_H

#include <inttypes.h>
#include <glad/glad.h>
#include <linmath.h>
#include "../object/chunk/chunk.h"

typedef struct Scene Scene;

typedef struct Camera Camera;

typedef struct Window Window;

typedef struct WindowRender WindowRender;
struct WindowRender
{
    int32_t ray_march_compute_shader;
    int32_t render_shader;
    int32_t shader_screen;

    uint32_t screen_vao;
    uint32_t screen_vbo;

    // We only use mesh
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
};

struct AWindowRender
{
    WindowRender *(*Init)();
    void (*Destroy)(WindowRender *render);

    void (*RenderScreenInit)();
    void (*RenderScreen)(Camera *camera);
    void (*RenderChunks)(Scene *scene, Camera *camera);
    void (*RayMarchChunkHeightTexture)(Scene *scene, Camera *camera);

    // Render
    void (*RenderBegin)(Window *window, Camera *camera);
    void (*RenderEnd)(Window *window);

    void (*RenderLight)(Window *window, vec3 position);
};

extern struct AWindowRender AWindowRender[1];

#endif