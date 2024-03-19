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
#include <linmath.h>
#include "../object/model/model.h"

typedef struct Window Window;

typedef struct WindowRender WindowRender;
struct WindowRender
{
    int32_t shader;

    // We only use mesh
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
};

struct AWindowRender
{
    WindowRender *(*Init)();
    void (*Destroy)(WindowRender *render);

    void (*RenderInitMesh)(WindowRender *render);
    void (*RenderMesh)(Model *model, mat4x4 transform);

    // Render
    void (*RenderBegin)(Window *window);
    void (*RenderEnd)(Window *window);
};

extern struct AWindowRender AWindowRender[1];

#endif