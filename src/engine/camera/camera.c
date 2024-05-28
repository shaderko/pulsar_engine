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
#include "../object/map/scene.h"

static Camera *Init()
{
    Camera *camera = malloc(sizeof(Camera));
    if (!camera)
        ERROR_EXIT("error allocating memory for camera.\n");

    memset(camera, 0, sizeof(Camera));

    memcpy(camera->position, (vec3){0, 0, 0}, sizeof(vec3));

    // Create matrixes for camera view and projection
    mat4x4_identity(camera->view);
    mat4x4_identity(camera->projection);

    // Set up necessary data for view
    memcpy(camera->center, (vec3){0.0f, 0.0f, 0}, sizeof(vec3));
    memcpy(camera->eye, (vec3){0.0f, 0.0f, 0}, sizeof(vec3));
    memcpy(camera->up, (vec3){0.0f, 1.0f, 0}, sizeof(vec3));

    // Initialize rendering to texture
    glGenTextures(1, &camera->image_out);
    glBindTexture(GL_TEXTURE_2D, camera->image_out);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
    // camera->last_width = 1920;
    // camera->last_height = 1080;
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create depth buffer texture
    glGenTextures(1, &camera->depth);
    glBindTexture(GL_TEXTURE_2D, camera->depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindImageTexture(0, camera->image_out, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // glGenRenderbuffers(1, &camera->depth);
    // glBindRenderbuffer(GL_RENDERBUFFER, camera->depth);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1920, 1080);

    glGenFramebuffers(1, &camera->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, camera->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, camera->image_out, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, camera->depth, 0);

    // Check if FBO creation was successful
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Error: Framebuffer is not complete!\n");
        // TODO:
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        fprintf(stderr, "[ERROR] Initializing camera: %d\n", error);
    }

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

static void UpdateView(Camera *camera)
{
    if (!camera)
    {
        puts("NO CAMERA");
        return;
    }

    memcpy(camera->eye, camera->position, sizeof(vec3));

    mat4x4_look_at(camera->view, camera->eye, camera->center, camera->up);
}

static void Render(Camera *camera, Window *window, int width, int height, Scene *scene)
{
    if (!camera)
        ERROR_RETURN(NULL, "[ERROR] Camera is null.");

    if (camera->last_width != width || camera->last_height != height)
    {
        // Update texture size
        // glBindTexture(GL_TEXTURE_2D, camera->image_out);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, camera->image_out);

    glBindFramebuffer(GL_FRAMEBUFFER, camera->fbo);

    // AWindowRender->RenderBegin(window, camera);
    glViewport(0, 0, 1920, 1080);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        fprintf(stderr, "OpenGL Camera Error: %d\n", error);
    }

    // glEndQuery(GL_TIME_ELAPSED);
    // GLuint64 timeElapsed;
    // glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &timeElapsed);
    // printf("Time taken before render in camera: %f ms\n", timeElapsed / 1000000.0);

    AScene.Render(scene, camera, width, height);

    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        fprintf(stderr, "OpenGL Camera 2 Error: %d\n", error);
    }

    camera->last_width = width;
    camera->last_height = height;

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