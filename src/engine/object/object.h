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
    vec3 position;
    vec3 velocity;
    float mass;
    bool is_static;
    bool should_render;
    SerializedCollider collider;
    SerializedRenderer renderer;
};

typedef struct Object Object;
struct Object
{
    ull id; // 8 bytes

    /**
     * Vector 3 position of object in world space
     */
    mat4x4 transform; // 64 bytes

    /**
     * Vector 3 velocity in each direction
     */
    vec3 velocity; // 12 bytes

    /**
     * Mass represents the speed at which object falls,
     * and how much velocity is transfered/lost on collision
     */
    float mass; // 4 bytes

    /**
     * Is a static or dynamic object (Doesn't move or does)
     */
    bool is_static; // 1 byte

    bool should_render; // 1 byte

    /**
     * Collider of object
     */
    Collider *collider;

    /**
     * Object renderer
     */
    Renderer *renderer;
};

struct AObject
{
    /**
     * Create object
     *
     * Can have different collider and renderer types
     */
    Object *(*Init)();

    Object *(*Create)(bool is_static, bool should_render, float mass, vec3 position);

    /**
     * Create an object box
     */
    Object *(*InitBox)(bool is_static, bool should_render, float mass, vec3 position, vec3 size);

    Object *(*InitMesh)(bool is_static, bool should_render, float mass, vec3 position, vec3 size, Model *model);

    Object *(*GetObjectByIndex)(int index);

    /**
     * Render object in position of object with the local position of renderer
     */
    void (*Render)(Object *object);
    void (*RenderObjects)();

    /**
     * Updates the position of an object with objects velocity
     */
    void (*Update)(Object *object);
    void (*UpdateObjects)();

    void (*UpdatePosition)(Object *object, vec3 position);

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