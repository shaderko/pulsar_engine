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
#include <glad/glad.h>
#include "../render/render.h"

// Defined for circular import
typedef struct Scene Scene;

typedef struct Camera Camera;
struct Camera
{
    vec3 position;

    mat4x4 view;
    mat4x4 projection;

    vec3 center;
    vec3 eye;
    vec3 up;

    // Used by the window to show the rendered image
    GLuint image_out;
    GLuint depth;
    GLuint fbo;

    // Save last render size
    int last_width;
    int last_height;
};

struct ACamera
{
    // Initialize Ortographic view camera
    Camera *(*InitOrtho)(float left, float right, float bottom, float top, float near, float far);

    // Initialize Perspective view camera
    Camera *(*InitPerspective)(float fov, float aspect, float near, float far);

    // Update camera view matrix, call each time the camera moves
    void (*UpdateView)(Camera *camera);

    // Render the camera view
    void (*Render)(Camera *camera, Window *window, int width, int height, Scene *scene);
};

extern struct ACamera ACamera[1];

#endif