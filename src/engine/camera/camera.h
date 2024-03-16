/**
 * @file camera.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <linmath.h>
#include "../object/object.h"

typedef struct Camera Camera;
struct Camera
{
    vec3 position;
    vec3 velocity;
    float distance;
    float smoothing;
    Object *target;
    mat4x4 view;
    mat4x4 projection;
};

struct ACamera
{
    Camera *(*Init)(float distance, float smoothing, vec3 position, Object *target);

    /**
     * @brief Update view matrix, used for rendering, so update after movement
     */
    void (*UpdateView)();

    /**
     * @brief Follow target, target must be set manually before calling this function
     */
    void (*FollowTarget)(vec2 *mousePos);

    /**
     * @brief Update position of the camera, also updates view matrix
     */
    void (*UpdatePosition)(vec3 position);
};

extern struct ACamera ACamera[1];

#endif