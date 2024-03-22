/**
 * @file game_object.c
 * @author https://github.com/shaderko
 * @brief An object is used to simulate physics and collisions, and is the main object in the game
 * @date 2023-04-15
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "object.h"

#include <glad/glad.h>
#include "../util/util.h"
#include "collider/collider.h"
#include "renderer/renderer.h"
#include "map/scene.h"

static ObjectGroup **object_group_list = NULL;
static size_t object_group_list_size = 0;

static ObjectGroup *init_group()
{
    ObjectGroup *object_group = malloc(sizeof(ObjectGroup));
    if (!object_group)
        ERROR_EXIT("Object group couldn't be allocated!\n");

    memset(object_group, 0, sizeof(ObjectGroup));

    object_group_list = realloc(object_group_list, sizeof(ObjectGroup *) * (object_group_list_size + 1));
    object_group_list[object_group_list_size] = object_group;
    object_group_list_size++;

    return object_group;
}

static void add_to_group(Object *object)
{
    for (int i = 0; i < object_group_list_size; i++)
    {
        if (object_group_list[i]->size <= 0)
        {
            // object_group_delete(object_group_list[i]);
        }

        ObjectGroup *group = object_group_list[i];

        if (group->objects[0]->renderer->model->id != object->renderer->model->id)
            continue;

        if (group->size <= group->index + 1)
        {
            size_t new_size = group->size * 2;
            // realloc objects array
            group->objects = realloc(group->objects, sizeof(Object *) * new_size);
            if (!group->objects)
                ERROR_EXIT("Couldn't allocate memory for object group!\n");

            // realloc vbo array

            // Create temp buffer to copy data from original buffer
            GLuint tempBuffer;
            glGenBuffers(1, &tempBuffer);
            glBindBuffer(GL_COPY_WRITE_BUFFER, tempBuffer);

            glBufferData(GL_COPY_WRITE_BUFFER, sizeof(mat4x4) * new_size, NULL, GL_STATIC_DRAW);
            glBindBuffer(GL_COPY_READ_BUFFER, group->vbo);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(mat4x4) * group->index);

            glDeleteBuffers(1, &group->vbo);
            group->vbo = tempBuffer;
            group->size = new_size;
        }

        group->objects[group->index] = object;

        glBindBuffer(GL_ARRAY_BUFFER, group->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(mat4x4) * group->index, sizeof(mat4x4), &object->transform[0][0]);

        group->index++;

        return;
    }

    ObjectGroup *group = init_group();

    group->objects = malloc(sizeof(Object *) * 1);
    group->objects[0] = object;

    glGenBuffers(1, &group->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, group->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mat4x4), &object->transform[0][0], GL_STATIC_DRAW);

    group->size = 1;
    group->index = 1;
}

/**
 * @brief Initialize an object with position
 *
 * @param position - in world space
 *
 * @return Object* - Pointer to the newly created object
 */
static Object *Init(vec3 position, vec3 rotation, vec3 scale)
{
    Object *object = malloc(sizeof(Object));
    if (!object)
        ERROR_EXIT("Object memory couldn't be allocated!\n");

    memset(object, 0, sizeof(Object));

    object->id = generate_random_id();

    mat4x4_identity(object->transform);

    mat4x4_translate_in_place(object->transform, position[0], position[1], position[2]);

    // Then apply rotations around axes
    mat4x4_rotate_X(object->transform, object->transform, rotation[0]); // Rotate around X axis
    mat4x4_rotate_Y(object->transform, object->transform, rotation[1]); // Rotate around Y axis
    mat4x4_rotate_Z(object->transform, object->transform, rotation[2]); // Rotate around Z axis

    // Finally, apply scale
    mat4x4_scale_aniso(object->transform, object->transform, scale[0], scale[1], scale[2]);

    return object;
}

/**
 * @brief Delete an object
 *
 * @param object - Pointer to the object to be freed
 *
 * @return void
 */
static void Delete(Object *object)
{
    // ACollider->Delete(object->collider);
    AObject.ARenderer->Delete(object->renderer);

    free(object);
}

/**
 * @brief Create a box object
 *
 * @param position - in world space
 * @param rotation - in world space
 * @param scale - in world space
 *
 * @return Object*
 */
static Object *InitBox(vec3 position, vec3 rotation, vec3 scale)
{
    Object *object = AObject.Init(position, rotation, scale);

    // object->collider = ACollider->InitBox((vec3){0, 0, 0}, size);
    object->renderer = AObject.ARenderer->InitBox((vec3){0, 0, 0}, (vec3){0, 0, 0}, (vec3){1, 1, 1});

    add_to_group(object);

    return object;
}

/**
 * @brief Create a mesh object
 *
 * @param position - in world space
 * @param rotation - in world space
 * @param scale - in world space
 * @param model - the model to be used for rendering
 *
 * @return Object*
 */
static Object *InitMesh(vec3 position, vec3 rotation, vec3 scale, Model *model)
{
    if (!model)
        ERROR_RETURN(NULL, "Object was not initialized because the model was not provided!\n");

    Object *object = AObject.Init(position, rotation, scale);

    // object->collider = ACollider->InitBox((vec3){0, 0, 0}, size);
    object->renderer = AObject.ARenderer->InitMesh(model, (vec3){0, 0, 0}, (vec3){0, 0, 0}, (vec3){1, 1, 1});

    add_to_group(object);

    return object;
}

/**
 * @brief Render a single object
 *
 * @param object object to be rendered
 */
static void Render(Object *object)
{
    AObject.ARenderer->Render(object->renderer, object->transform);
}

/**
 * @brief Render multiple objects efficiently
 *
 * @param objects
 * @param objects_size
 */
static void BatchRender()
{
    for (int i = 0; i < object_group_list_size; i++)
    {
        ObjectGroup *group = object_group_list[i];

        if (group->index <= 0)
            continue;

        // printf("rendering object %lld\n", object_group_list[i]->objects[0]->id);

        // Render(object_group_list[i]->objects[0]);
        AObject.ARenderer->BatchRender(group->objects[0]->renderer->model, group->vbo, group->index);
    }
}

/**
 * @brief Update objects position
 *
 * @param object
 * @param position
 */
static void Translate(Object *object, vec3 position)
{
    mat4x4_translate(object->transform, position[0], position[1], position[2]);
}

/**
 * @brief Serialize the object for saving to file or network transfer
 *
 * @param object
 * @return SerializedDerived
 */
static SerializedDerived Serialize(Object *object)
{
    // SerializedCollider collider = ACollider->Serialize(object->collider);
    // SerializedRenderer renderer = AObject.ARenderer->Serialize(object->renderer);

    // SerializedObject serialize_obj = {
    //     object->id,
    //     {0, 0, 0},
    //     // {object->position[0], object->position[1], object->position[2]},
    //     {object->velocity[0], object->velocity[1], object->velocity[2]},
    //     object->mass,
    //     object->is_static,
    //     object->should_render,
    //     collider,
    //     renderer};

    // SerializedDerived result;
    // result.len = sizeof(SerializedObject) + collider.derived.len + renderer.derived.len;
    // result.data = malloc(result.len);
    // if (!result.data)
    // {
    //     free(collider.derived.data);
    //     free(renderer.derived.data);
    //     ERROR_EXIT("SerializedDerived memory couldn't be allocated!\n");
    // }

    // memcpy((char *)result.data, &serialize_obj, sizeof(SerializedObject));
    // memcpy((char *)result.data + sizeof(SerializedObject), (char *)collider.derived.data, collider.derived.len);

    // // Renderer
    // memcpy((char *)result.data + sizeof(SerializedObject) + collider.derived.len, (char *)renderer.derived.data, renderer.derived.len);

    // free(collider.derived.data);
    // free(renderer.derived.data);

    SerializedDerived result;

    return result;
}

/**
 * Serialize partially so it doesn't include the renderer or collider, to be faster for network transfer TODO:
 * @param object
 * @return SerializedDerived
 */
static SerializedDerived SerializePartial(Object *object)
{
    // SerializedObject serialize_obj = {
    //     object->id,
    //     {0, 0, 0},
    //     // {object->position[0], object->position[1], object->position[2]},
    //     {object->velocity[0], object->velocity[1], object->velocity[2]},
    //     object->mass,
    //     object->is_static,
    //     object->should_render,
    //     {{0, 0, 0}, 0, {0, NULL}},
    //     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, NULL}}};

    // SerializedDerived result;
    // result.len = sizeof(SerializedObject);
    // result.data = malloc(result.len);
    // if (!result.data)
    //     ERROR_EXIT("SerializedDerived memory couldn't be allocated!\n");

    // memcpy((char *)result.data, &serialize_obj, sizeof(SerializedObject));

    SerializedDerived result;

    return result;
}

/**
 * @brief Deserialize the object, update it's variables and create a new one if it doesn't exist
 *
 * @param object
 * @param collider
 * @param renderer
 * @return Object*
 */
static Object *Deserialize(SerializedObject *object, Scene *scene)
{
    printf("Deserializing game object with id %llu\n", object->id);
    // if (!scene)
    // {
    //     for (int x = 0; x < ObjectsSize; x++)
    //     {
    //         if (ObjectsArray[x]->id == object->id)
    //         {
    //             // ObjectsArray[x]->position[0] = object->position[0];
    //             // ObjectsArray[x]->position[1] = object->position[1];
    //             // ObjectsArray[x]->position[2] = object->position[2];
    //             ObjectsArray[x]->velocity[0] = object->velocity[0];
    //             ObjectsArray[x]->velocity[1] = object->velocity[1];
    //             ObjectsArray[x]->velocity[2] = object->velocity[2];
    //             return NULL;
    //         }
    //     }
    // }
    // else
    // {
    //     for (int x = 0; x < scene->objects_size; x++)
    //     {
    //         if (scene->objects[x]->id == object->id)
    //         {
    //             // scene->objects[x]->position[0] = object->position[0];
    //             // scene->objects[x]->position[1] = object->position[1];
    //             // scene->objects[x]->position[2] = object->position[2];
    //             scene->objects[x]->velocity[0] = object->velocity[0];
    //             scene->objects[x]->velocity[1] = object->velocity[1];
    //             scene->objects[x]->velocity[2] = object->velocity[2];
    //             return NULL;
    //         }
    //     }
    // }

    // Object *new_obj = AObject.Create(object->is_static, object->should_render, object->mass, object->position);
    // new_obj->id = object->id;
    // switch (object->collider.type)
    // {
    // case BOX_COLLIDER:
    //     new_obj->collider = ACollider->InitBox((float *)object->collider.position, (float *)&(vec3){100, 100, 100});
    //     break;
    // default:
    //     break;
    // }

    // printf("object->renderer.derived.len: %d\n", object->renderer.derived.len);

    // new_obj->renderer = AObject.ARenderer->Deserialize(object->renderer);

    Object *new_obj = AObject.Init((vec3){0, 0, 0}, (vec3){0, 0, 0}, (vec3){0, 0, 0});

    return new_obj;
}

extern struct ARenderer ARenderer;
struct AObject AObject =
    {
        .Init = Init,
        .Delete = Delete,
        .InitBox = InitBox,
        .InitMesh = InitMesh,
        .Render = Render,
        .BatchRender = BatchRender,
        .Translate = Translate,
        .Serialize = Serialize,
        .SerializePartial = SerializePartial,
        .Deserialize = Deserialize,
        .ARenderer = &ARenderer,
};