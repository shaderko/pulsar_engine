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
#include "../object/object.h"

static Camera *Init()
{
    Camera *camera = malloc(sizeof(Camera));
    if (!camera)
        ERROR_EXIT("error allocating memory for camera.\n");

    memcpy(camera->position, (vec3){0, 0, 0}, sizeof(vec3));

    mat4x4_identity(camera->view);
    mat4x4_identity(camera->projection);

    memcpy(camera->center, (vec3){0.0f, 0.0f, 0}, sizeof(vec3));
    memcpy(camera->eye, (vec3){0.0f, 0.0f, 0}, sizeof(vec3));
    memcpy(camera->up, (vec3){0.0f, 1.0f, 0}, sizeof(vec3));

    // Initialize rendering to texture
    glGenTextures(1, &camera->color);
    glBindTexture(GL_TEXTURE_2D, camera->color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &camera->depth);
    glBindRenderbuffer(GL_RENDERBUFFER, camera->depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, 1920, 1080);

    // Create low-resolution FBO and attach color texture and depth renderbuffer
    glGenFramebuffers(1, &camera->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, camera->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, camera->color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, camera->depth);

    // Check if FBO creation was successful
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Error: Framebuffer is not complete!\n");
        // TODO:
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return camera;
}

static Camera *InitOrtho(float left, float right, float bottom, float top, float near, float far)
{
    Camera *camera = Init();

    mat4x4_ortho(camera->projection, left, right, bottom, top, near, far);

    return camera;
}

static Camera *InitPerspective(float fov, float aspect, float near, float far)
{
    Camera *camera = Init();

    mat4x4_perspective(camera->projection, fov, aspect, near, far);

    return camera;
}

static Camera *UpdateView(Camera *camera)
{
    if (!camera)
        return;

    memcpy(camera->eye, camera->position, sizeof(vec3));

    mat4x4_look_at(camera->view, camera->eye, camera->center, camera->up);
}

static void Render(Camera *camera, Window *window, float width, float height)
{
    if (!camera)
        return;

    static float lastRenderedWidth;
    static float lastRenderedHeight;

    if (lastRenderedWidth != width || lastRenderedHeight != height)
    {
        // Update texture size
        glBindTexture(GL_TEXTURE_2D, camera->color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        // Update depth renderbuffer size
        glBindRenderbuffer(GL_RENDERBUFFER, camera->depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);

        // Re-attach updated texture and renderbuffer to FBO (not strictly necessary if the binding points haven't changed, but good for clarity)
        glBindFramebuffer(GL_FRAMEBUFFER, camera->fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, camera->color, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, camera->depth);

        // Check FBO status
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("Error: Resized Framebuffer is not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, camera->fbo);

    AWindowRender->RenderBegin(window);
    glViewport(0, 0, width, height);

    AObject.RenderObjects();
    // AWindowRender->RenderEnd(window);

    lastRenderedWidth = width;
    lastRenderedHeight = height;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void Delete(Camera *camera)
{
    if (!camera)
        return;

    free(camera);
}

struct ACamera ACamera[1] = {{InitOrtho,
                              InitPerspective,
                              UpdateView, Render}};