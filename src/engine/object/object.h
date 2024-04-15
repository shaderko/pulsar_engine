/**
 * @file game_object.h
 * @author https://github.com/shaderko
 * @brief Object that can have different types of renderer and collider, has collisions and gravity
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <SDL.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../util/util.h"
#include "collider/collider.h"
#include "renderer/renderer.h"

// Defined for circular import
typedef struct Scene Scene;

typedef struct SerializedObject SerializedObject;
struct SerializedObject
{
    ull id;
    mat4x4 transform;
    SerializedCollider collider;
    SerializedRenderer renderer;
};

typedef struct ObjectGroup ObjectGroup;
struct ObjectGroup
{
    Object **objects;
    GLuint vbo;
    SDL_mutex *mutex;

    size_t size;
    size_t index;
};

typedef struct Object Object;
struct Object
{
    // Unique id of object
    ull id; // 8 bytes

    // Transformation of object
    mat4x4 transform; // 64 bytes

    // Objects collider
    Collider *collider; // 8 bytes

    // Objects renderer
    Renderer *renderer; // 8 bytes
};

struct AObject
{
    // Initialize an object with null values
    Object *(*Init)(vec3 position, vec3 rotation, vec3 scale);

    void (*Delete)(Object *object);

    Object *(*InitBox)(vec3 position, vec3 rotation, vec3 scale);

    /**
     * Create an object box
     */
    Object *(*InitMesh)(vec3 position, vec3 rotation, vec3 scale, Model *model);

    void (*Render)(Object *object);

    void (*BatchRender)();

    void (*Translate)(Object *object, vec3 position);

    /**
     * Serialize an object for network transfer
     */
    SerializedDerived (*Serialize)(Object *object);
    SerializedDerived (*SerializePartial)(Object *object);

    /**
     * Deserialize network Object
     * Create new one if it doesn't exist
     * If it does apply all variables
     */
    Object *(*Deserialize)(SerializedObject *object, Scene *scene);

    // struct ACollider *ACollider;
    struct ARenderer *ARenderer;
};

extern struct AObject AObject;

#endif