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

#include "../util/util.h"
#include "collider/collider.h"
#include "renderer/renderer.h"
#include "map/scene.h"

static Object **ObjectsArray = NULL;
static size_t ObjectsSize = 0;

/**
 * @return Object* - pointer to object in array
 */
static Object *Init()
{
    Object *object = malloc(sizeof(Object));
    if (!object)
        ERROR_EXIT("Object memory couldn't be allocated!\n");

    memset(object, 0, sizeof(Object));

    object->id = generate_random_id();
    printf("Initialized object with id %llu\n", object->id);

    ObjectsArray = realloc(ObjectsArray, (ObjectsSize + 1) * sizeof(Object *));
    if (!ObjectsArray)
    {
        ERROR_EXIT("ObjectsArray memory couldn't be allocated!\n");
    }

    ObjectsArray[ObjectsSize] = object;
    ObjectsSize++;

    return object;
}

static void Delete(Object *object)
{
    ACollider->Delete(object->collider);
    AObject.ARenderer->Delete(object->renderer);

    for (int i = 0; i < ObjectsSize; i++)
    {
        if (ObjectsArray[i] == object)
        {
            ObjectsArray[i] = NULL;
            ObjectsArray[i] = ObjectsArray[ObjectsSize - 1];
            ObjectsArray = realloc(ObjectsArray, (ObjectsSize - 1) * sizeof(Object *));
            break;
        }
    }
    ObjectsSize--;

    free(object);
}

/**
 * Initialize an object with no collider and renderer, but with all the other variables
 *
 * @param is_static dynamic physics or static
 * @return Object*
 */
static Object *Create(bool is_static, bool should_render, float mass, vec3 position)
{
    Object *object = Init();

    /**
     * Initialize all the main variables
     */
    mat4x4_translate(object->transform, position[0], position[1], position[2]);
    object->mass = mass;
    object->is_static = is_static;
    object->should_render = should_render;

    return object;
}

/**
 * @brief Create a box object
 *
 * @param is_static dynamic or static physics
 * @return Object*
 */
static Object *InitBox(bool is_static, bool should_render, float mass, vec3 position, vec3 size)
{
    Object *object = AObject.Create(is_static, should_render, mass, position);

    // object->collider = ACollider->InitBox((vec3){0, 0, 0}, size);
    object->renderer = AObject.ARenderer->InitBox((vec3){0, 0, 0}, (vec3){0, 0, 0}, size);

    return object;
}

/**
 * @brief Create a mesh object
 * If model is NULL it will error exit the program
 *
 * @param is_static
 * @param mass
 * @param position
 * @param size
 * @param model
 * @return Object*
 */
static Object *InitMesh(bool is_static, bool should_render, float mass, vec3 position, vec3 size, Model *model)
{
    if (!model)
        ERROR_RETURN(NULL, "Object was not initialized because the model was NULL!\n");

    Object *object = AObject.Create(is_static, should_render, mass, position);

    // object->collider = ACollider->InitBox((vec3){0, 0, 0}, size);
    object->renderer = AObject.ARenderer->InitMesh(model, (vec4){1, 1, 1, 1}, (vec3){0, 0, 0}, (vec3){0, 0, 0}, size);

    return object;
}

/**
 * @brief Get the object by index of object in array
 *
 * @param index index of the object in the array
 * @return Object* or NULL if the object doesn't exist
 */
static Object *GetObjectByIndex(int index)
{
    if (index >= ObjectsSize || index < 0)
    {
        return NULL;
    }
    return ObjectsArray[index];
}

/**
 * @brief Render the object
 *
 * @param object object to be rendered
 */
static void Render(Object *object)
{
    AObject.ARenderer->Render(object->renderer, object->transform);
}

static void BatchRender(Object **objects, size_t objects_size)
{
    // Create list of renderers from objects
    Object **renderers = malloc(objects_size * sizeof(Renderer *));
    mat4x4 *transforms = malloc(objects_size * sizeof(mat4x4));
    for (int i = 0; i < objects_size; i++)
    {
        renderers[i] = objects[i]->renderer;
        memcpy(transforms[i], objects[i]->transform, sizeof(mat4x4));
    }

    AObject.ARenderer->BatchRender(renderers, transforms, objects_size);

    free(renderers);
    free(transforms);
}

/**
 * @brief Render all objects
 */
static void RenderObjects(Scene *scene)
{
    if (!scene)
        return;

    for (int x = 0; x < scene->objects_list_size; x++)
    {
        // if (!ObjectsArray[x]->should_render)
        //     continue;
        // Render(ObjectsArray[x]);

        BatchRender(scene->objects_list[x]->object, scene->objects_list[x]->object_size);
    }
}

/**
 * @brief Apply gravity to object
 * Doesn't check for collisions
 *
 * @param object to apply gravity to
 */
static void ApplyGravity(Object *object)
{
    object->velocity[1] += -.01;
}

/**
 * @brief Update the game object and it's physics and collisions
 *
 * @param object
 */
static void Update(Object *object)
{
    if (object->is_static)
    {
        return;
    }

    for (int i = 0; i < ObjectsSize; i++)
    {
        if (object->collider->Collide(object, ObjectsArray[i]))
        {
            return;
        }
    }

    ApplyGravity(object);

    // object->position[0] += object->velocity[0];
    // object->position[1] += object->velocity[1];
    // object->position[2] += object->velocity[2];
}

/**
 * @brief Update all objects
 */
static void UpdateObjects()
{
    for (int x = 0; x < ObjectsSize; x++)
    {
        Update(ObjectsArray[x]);
    }
}

/**
 * @brief Update position not depending on velocity of the object
 *
 * @param object
 * @param position
 */
static void UpdatePosition(Object *object, vec3 position)
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
    SerializedCollider collider = ACollider->Serialize(object->collider);
    SerializedRenderer renderer = AObject.ARenderer->Serialize(object->renderer);

    SerializedObject serialize_obj = {
        object->id,
        {0, 0, 0},
        // {object->position[0], object->position[1], object->position[2]},
        {object->velocity[0], object->velocity[1], object->velocity[2]},
        object->mass,
        object->is_static,
        object->should_render,
        collider,
        renderer};

    SerializedDerived result;
    result.len = sizeof(SerializedObject) + collider.derived.len + renderer.derived.len;
    result.data = malloc(result.len);
    if (!result.data)
    {
        free(collider.derived.data);
        free(renderer.derived.data);
        ERROR_EXIT("SerializedDerived memory couldn't be allocated!\n");
    }

    memcpy((char *)result.data, &serialize_obj, sizeof(SerializedObject));
    memcpy((char *)result.data + sizeof(SerializedObject), (char *)collider.derived.data, collider.derived.len);

    // Renderer
    memcpy((char *)result.data + sizeof(SerializedObject) + collider.derived.len, (char *)renderer.derived.data, renderer.derived.len);

    free(collider.derived.data);
    free(renderer.derived.data);

    return result;
}

/**
 * Serialize partially so it doesn't include the renderer or collider, to be faster for network transfer TODO:
 * @param object
 * @return SerializedDerived
 */
static SerializedDerived SerializePartial(Object *object)
{
    SerializedObject serialize_obj = {
        object->id,
        {0, 0, 0},
        // {object->position[0], object->position[1], object->position[2]},
        {object->velocity[0], object->velocity[1], object->velocity[2]},
        object->mass,
        object->is_static,
        object->should_render,
        {{0, 0, 0}, 0, {0, NULL}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, NULL}}};

    SerializedDerived result;
    result.len = sizeof(SerializedObject);
    result.data = malloc(result.len);
    if (!result.data)
        ERROR_EXIT("SerializedDerived memory couldn't be allocated!\n");

    memcpy((char *)result.data, &serialize_obj, sizeof(SerializedObject));

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

    Object *new_obj = AObject.Create(object->is_static, object->should_render, object->mass, object->position);
    new_obj->id = object->id;
    switch (object->collider.type)
    {
    case BOX_COLLIDER:
        new_obj->collider = ACollider->InitBox((float *)object->collider.position, (float *)&(vec3){100, 100, 100});
        break;
    default:
        break;
    }

    printf("object->renderer.derived.len: %d\n", object->renderer.derived.len);

    new_obj->renderer = AObject.ARenderer->Deserialize(object->renderer);

    return new_obj;
}

extern struct ARenderer ARenderer;
struct AObject AObject =
    {
        .Init = Init,
        .Delete = Delete,
        .Create = Create,
        .InitBox = InitBox,
        .InitMesh = InitMesh,
        .GetObjectByIndex = GetObjectByIndex,
        .Render = Render,
        .BatchRender = BatchRender,
        .RenderObjects = RenderObjects,
        .Update = Update,
        .UpdateObjects = UpdateObjects,
        .UpdatePosition = UpdatePosition,
        .Serialize = Serialize,
        .SerializePartial = SerializePartial,
        .Deserialize = Deserialize,
        .ARenderer = &ARenderer,
};