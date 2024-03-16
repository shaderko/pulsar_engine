/**
 * @file box_collider.h
 * @author https://github.com/shaderko
 * @brief Box collider used by Collider class
 * @version 0.1
 * @date 2023-04-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef BOX_COLLIDER_H
#define BOX_COLLIDER_H

#include <stdbool.h>
#include <linmath.h>

#include "collider.h"

typedef struct BoxCollider BoxCollider;
struct BoxCollider
{
    /**
     * Collider class, parent to Box Collider
     */
    Collider *parent;

    /**
     * Vector 3 size of Box Collider
     */
    vec3 size;
};

struct ABoxCollider
{
    /**
     * Create box collider
     */
    Collider *(*Init)(Collider *collider, vec3 size);

    /**
     * Delete box collider
     */
    void (*Delete)(Collider *collider);

    /**
     * Get the Size of a box collider
     */
    vec3 *(*Size)(Collider *collider);

    /**
     * Check if two box colliders are colliding
     */
    bool (*Collide)(Object *object1, Object *object2);

    /**
     * Serialize box collider
     */
    SerializedDerived (*Serialize)(Collider *collider);
};

extern struct ABoxCollider ABoxCollider[1];

#endif