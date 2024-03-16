/**
 * @file collider.h
 * @author https://github.com/shaderko
 * @brief Collisions used for physics
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef COLLIDER_H
#define COLLIDER_H

#include <linmath.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "../serialized/serialized.h"

typedef struct Object Object;

typedef enum ColliderType ColliderType;
enum ColliderType
{
    NONE_COLLIDER,
    BOX_COLLIDER,
};

typedef struct SerializedCollider SerializedCollider;
struct SerializedCollider
{
    vec3 position;
    ColliderType type;
    SerializedDerived derived;
};

typedef struct Collider Collider;
struct Collider
{
    /**
     * Pointer to the subclass
     */
    void *derived;

    /**
     * Local position to game object
     */
    vec3 position;

    /**
     * Collider type for serialization
     */
    ColliderType type;

    bool (*Collide)(Object *, Object *);
    void (*Delete)(Collider *);
    vec3 *(*Size)(Collider *);
    SerializedDerived (*Seralize)(Collider *);
};

struct ACollider
{
    /**
     * Create a new collider
     */
    Collider *(*Init)();

    /**
     * Create a new box collider
     */
    Collider *(*InitBox)(vec3 position, vec3 size);

    /**
     * This function is only used if the collider has no derived class
     */
    void (*Delete)(Collider *collider);

    /**
     * Serialize the collider
     */
    SerializedCollider (*Serialize)(Collider *);
};

extern struct ACollider ACollider[1];

#endif