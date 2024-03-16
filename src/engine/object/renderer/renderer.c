/**
 * @file renderer.c
 * @author https://github.com/shaderko
 * @brief Used to render a model
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "renderer.h"

#include "../../util/util.h"
#include "../../render/render.h"

/**
 * Delete the renderer
 * @param renderer - to be deleted
 * This function frees all the memory a renderer uses, also the model
 * TODO: adjust to not delete the model, because in a game there will be multiple renderers with the same model, all can use the same one, so model should be freed separately
 */
static void Delete(Renderer *renderer)
{
    AModel->Delete(renderer->model);
    free(renderer);
}

/**
 * @brief Initialize a renderer
 *
 * @param position - local to object
 * @param rotation  - local to object
 * @param scale - local to object
 * @return Renderer*
 */
static Renderer *Init(vec3 position, vec3 rotation, vec3 scale)
{
    Renderer *renderer = malloc(sizeof(Renderer));
    if (!renderer)
    {
        ERROR_EXIT("error allocating memory for collider.");
    }

    memcpy(renderer->position, position, sizeof(vec3));
    memcpy(renderer->rotation, rotation, sizeof(vec3));
    memcpy(renderer->scale, scale, sizeof(vec3));

    return renderer;
}

/**
 * @brief Render the renderer, if model is NULL, then it won't render, the position is added to the position of the object
 *
 * @param renderer
 * @param position
 */
static void Render(Renderer *renderer, vec3 position)
{
    if (!renderer || !renderer->model)
    {
        printf("Couldn't render, renderer NULL, or model NULL!\n");
        return;
    }

    // Adjust the position to be relative to world space by adding the position of the renderer to the position of the object
    vec3 world_position;
    vec3_add(world_position, position, renderer->position);

    AWindowRender->RenderMesh(renderer->model, world_position, renderer->scale); // renderer->rotation, renderer->scale TODO:
}

/**
 * @brief Initialize a cube renderer
 *
 * @param position - vec3 of renderer position
 * @param rotation - vec3 of renderer rotation
 * @param scale - vec3 of renderer scale
 * @return Renderer*
 */
static Renderer *InitBox(vec3 position, vec3 rotation, vec3 scale)
{
    Renderer *renderer = Init(position, rotation, scale);

    renderer->model = AModel->InitBox();

    return renderer;
}

/**
 * @brief Initialize a mesh renderer
 *
 * @param model - initialized model
 * @param color - color of the mesh // TODO: do this on the upper function as well, and also this attribute is not being used
 * @param position - vec3 of renderer position
 * @param rotation - vec3 of renderer rotation
 * @param scale - vec3 of renderer scale
 * @return Renderer*
 */
static Renderer *InitMesh(Model *model, vec4 color, vec3 position, vec3 rotation, vec3 scale)
{
    Renderer *renderer = Init(position, rotation, scale);

    renderer->model = model;

    return renderer;
}

/**
 * @brief Serialize the renderer for saving to file or sending over network
 *
 * @param renderer
 * @return SerializedRenderer
 */
static SerializedRenderer Serialize(Renderer *renderer)
{
    SerializedRenderer serialized = {0};

    memcpy(serialized.position, renderer->position, sizeof(vec3));
    memcpy(serialized.rotation, renderer->rotation, sizeof(vec3));
    memcpy(serialized.scale, renderer->scale, sizeof(vec3));

    serialized.derived = AModel->Serialize(renderer->model);

    return serialized;
}

/**
 * @brief Deserialize the renderer from file or network
 *
 * @param serialized_renderer
 * @return Renderer*
 */
static Renderer *Deserialize(SerializedRenderer serialized_renderer)
{
    Renderer *renderer = malloc(sizeof(Renderer));

    memcpy(renderer->position, serialized_renderer.position, sizeof(vec3));
    memcpy(renderer->rotation, serialized_renderer.rotation, sizeof(vec3));
    memcpy(renderer->scale, serialized_renderer.scale, sizeof(vec3));

    renderer->model = AModel->Deserialize(serialized_renderer.derived);

    return renderer;
}

struct ARenderer ARenderer = {
    .Init = Init,
    .Render = Render,
    .InitBox = InitBox,
    .InitMesh = InitMesh,
    .Serialize = Serialize,
    .Deserialize = Deserialize,
};