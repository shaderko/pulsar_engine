/**
 * @file collider.c
 * @author https://github.com/shaderko
 * @brief Collider used for collisions
 * @version 0.1
 * @date 2023-04-14
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "../../util/util.h"
#include "collider.h"
#include "box_collider.h"

static Collider *Init()
{
    Collider *collider = malloc(sizeof(Collider));
    if (!collider)
    {
        ERROR_EXIT("Error allocating memory for collider.");
    }

    collider->Delete = ACollider->Delete;

    return collider;
}

static Collider *InitBox(vec3 position, vec3 size)
{
    Collider *collider = ACollider->Init();

    memcpy(collider->position, position, sizeof(vec3));
    ABoxCollider->Init(collider, size);

    return collider;
}

static void Delete(Collider *collider)
{
    if (!collider)
        return;

    collider->Delete(collider);

    free(collider);
}

static SerializedCollider Serialize(Collider *collider)
{
    SerializedCollider serialized = {
        {collider->position[0], collider->position[1], collider->position[2]},
        collider->type,
        collider->Seralize(collider),
    };
    return serialized;
}

struct ACollider ACollider[1] =
    {{
        Init,
        InitBox,
        Delete,
        Serialize,
    }};