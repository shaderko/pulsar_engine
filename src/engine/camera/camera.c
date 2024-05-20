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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, camera->image_out);

    // Some texture shenanigans
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Default the camera width and height to 1920x1080
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
    camera->last_width = 1920;
    camera->last_height = 1080;
    glBindImageTexture(0, camera->image_out, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // DEBUG
    // Initialize rendering to texture
    GLuint debug_image;
    glGenTextures(1, &debug_image);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, debug_image);

    // Some texture shenanigans
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Default the camera width and height to 1920x1080
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(3, debug_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // Initialize rendering to texture
    GLuint debug_image2;
    glGenTextures(1, &debug_image2);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, debug_image2);

    // Some texture shenanigans
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Default the camera width and height to 1920x1080
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(4, debug_image2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // Idk what this does (bind frame buffer to screen i think)
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
    // GLuint queries[2];
    // glGenQueries(2, queries);
    // glBeginQuery(GL_TIME_ELAPSED, queries[0]);

    if (!camera)
        ERROR_RETURN(NULL, "[ERROR] Camera is null.");

    float black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    // Clear the image with black
    glClearTexImage(camera->image_out, 0, GL_RGBA, GL_FLOAT, &black);

    if (camera->last_width != width || camera->last_height != height)
    {
        // Update texture size
        glBindTexture(GL_TEXTURE_2D, camera->image_out);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, camera->image_out);

    AWindowRender->RenderBegin(window, camera);
    glViewport(0, 0, width, height);

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

    // float *data = (float *)malloc(width * height * 4 * sizeof(float));
    // glBindTexture(GL_TEXTURE_2D, camera->image_out);
    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);

    camera->last_width = width;
    camera->last_height = height;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // free(data);
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