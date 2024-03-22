/**
 * @file renderer.h
 * @author https://github.com/shaderko
 * @brief Used to render a model
 * @version 0.1
 * @date 2023-07-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <inttypes.h>
#include <stdlib.h>
#include <linmath.h>

#include "../serialized/serialized.h"
#include "../model/model.h"
#include "../../util/util.h"

typedef struct SerializedRenderer SerializedRenderer;
struct SerializedRenderer
{
    vec3 position;
    vec3 rotation;
    vec3 scale;
    SerializedDerived derived;
};

typedef struct Renderer Renderer;
struct Renderer
{
    // Locale transform
    mat4x4 transform;

    // Model to render
    Model *model;
};

struct ARenderer
{
    /**
     * Initialize a renderer with values of position, rotation and scale
     * Important this function doesn't give the renderer a model, so it can't be rendered
     */
    Renderer *(*Init)(vec3 position, vec3 rotation, vec3 scale);

    void (*Delete)(Renderer *renderer);

    /**
     * Render the renderer
     */
    void (*Render)(Renderer *renderer, mat4x4 transform);

    void (*BatchRender)(Renderer **renderer, uint32_t vbo, size_t size);

    /**
     * Initialize a renderer with a box model
     */
    Renderer *(*InitBox)(vec3 position, vec3 rotation, vec3 scale);
    /**
     * Initialize a renderer with a mesh model
     */
    Renderer *(*InitMesh)(Model *model, vec3 position, vec3 rotation, vec3 scale);

    /**
     * Serialize a renderer, for saving to file or sending over network
     */
    SerializedRenderer (*Serialize)(Renderer *);
    /**
     * Deserialize a renderer, for loading from file or receiving over network
     */
    Renderer *(*Deserialize)(SerializedRenderer serialized);
};

#endif