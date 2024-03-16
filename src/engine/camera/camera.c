/**
 * @file camera.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "camera.h"
#include "../common/global/global.h"
#include "../util/util.h"

static Camera *camera = NULL;

static Camera *Init(float distance, float smoothing, vec3 position, Object *target)
{
    camera = malloc(sizeof(Camera));
    if (!camera)
    {
        ERROR_EXIT("error allocating memory for camera.\n");
    }

    camera->target = target;
    camera->distance = distance;
    camera->smoothing = smoothing;
    memcpy(camera->position, position, sizeof(vec3));
    mat4x4_identity(camera->view);
    ACamera->UpdateView();

    return camera;
}

static void UpdateView()
{
    int z_distance = 10000; // TODO:
    vec3 eye = {camera->position[0], z_distance + camera->position[1], z_distance * 2};
    vec3 center = {camera->position[0], camera->position[1], 0};
    vec3 up = {0.0f, 1.0f, 0};
    mat4x4_look_at(camera->view, eye, center, up);
}

/**
 * @brief Follow the target
 *
 * @param mousePos - (Optional) mouse position (vec2)
 */
static void FollowTarget(vec2 *mousePos)
{
    if (!camera->target)
    {
        return;
    }
    vec3 targetPos;
    vec3 playerPos = {camera->target->position[0], camera->target->position[1], 0};

    if (mousePos)
    {
        // vec3 targetMousePos = {((*mousePos)[0] - global.render.width / 2) / (global.render.width / 2), ((*mousePos)[1] - global.render.height / 2) / (global.render.height / 2), 0};
        // memcpy(targetPos, &(vec3){playerPos[0] + targetMousePos[0] * camera->distance, playerPos[1] + targetMousePos[1] * camera->distance, 0}, sizeof(vec3));
    }
    else
    {
        memcpy(targetPos, &(vec3){playerPos[0], playerPos[1], 0}, sizeof(vec3));
    }

    ACamera->UpdatePosition(targetPos);
}

static void UpdatePosition(vec3 position)
{
    camera->velocity[0] = (position[0] - camera->position[0]) / camera->smoothing;
    camera->velocity[1] = (position[1] - camera->position[1]) / camera->smoothing;

    camera->position[0] += camera->velocity[0];
    camera->position[1] += camera->velocity[1];

    ACamera->UpdateView();
}

struct ACamera ACamera[1] = {{
    Init,
    UpdateView,
    FollowTarget,
    UpdatePosition,
}};