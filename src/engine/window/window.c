/**
 * @file window.c
 * @author https://github.com/shaderko
 * @brief This file creates a new window and handles its events.
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <glad/glad.h>
#include <SDL.h>
#include <stdio.h>
#include <linmath.h>
#include "window.h"
#include "../util/util.h"

static Window *Init(int width, int height, char *title)
{
    Window *window = malloc(sizeof(Window));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        ERROR_EXIT("couldn't initialize SDL: %s\n", SDL_GetError());

// Decide GL+GLSL versions
#if __APPLE__
    // GL 4.1 Core + GLSL 410
    puts("Using OpenGL 4.1 Core and GLSL 4.1");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
#else
    puts("Using OpenGL 4.6 Core and GLSL 460");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    // and prepare OpenGL stuff
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    // SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    window->sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    window->width = width;
    window->height = height;

    if (!window->sdl_window)
    {
        ERROR_EXIT("window creation failed: %s\n", SDL_GetError());
    }

    window->renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_ACCELERATED);

    if (!window->renderer)
    {
        ERROR_EXIT("renderer creation failed: %s\n", SDL_GetError());
    }

    window->context = SDL_GL_CreateContext(window->sdl_window);
    if (!window->context)
    {
        ERROR_EXIT("failed to create window: %s\n", SDL_GetError());
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        ERROR_EXIT("failed to load GL: %s\n", SDL_GetError());
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    puts("openGL loaded");
    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    printf("vendor:        %s\n", glGetString(GL_VENDOR));
    printf("renderer:      %s\n", glGetString(GL_RENDERER));
    printf("version:       %s\n", glGetString(GL_VERSION));
    printf("glsl version:  %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    SDL_GL_MakeCurrent(window->sdl_window, window->context);
    window->render = AWindowRender->Init();

    return window;
}

static void Destroy(Window *window)
{
    if (window != NULL)
    {
        if (window->context != NULL)
        {
            SDL_GL_DeleteContext(window->context);
        }

        if (window->sdl_window != NULL)
        {
            SDL_DestroyWindow(window->sdl_window);
        }

        if (window->renderer != NULL)
        {
            SDL_DestroyRenderer(window->renderer);
        }

        SDL_Quit();

        AWindowRender->Destroy(window->render);

        free(window);
    }
}

static float radius = 50.0f;

static void Render(Window *window)
{
    if (!window || !window->camera)
        ERROR_RETURN(NULL, "[ERROR] Window is NULL.");

    static int lastX = 0, lastY = 0; // Static variables to remember the last position
    int x, y;
    SDL_GetMouseState(&x, &y);
    // if (editor->btn_pressed)
    {
        static float theta = 0;      // Azimuthal angle in radians
        static float phi = M_PI / 4; // Polar angle in radians (start with a 45-degree angle for elevation)

        int deltaX = x - lastX;
        int deltaY = y - lastY;

        lastX = x;
        lastY = y;

        float sensitivity = 0.005f; // Adjust this value to control the speed of the rotation
        theta += deltaX * sensitivity;
        phi += deltaY * sensitivity; // Subtracting so that dragging up moves the camera up

        // Clamp phi to prevent the camera from flipping over at the poles
        phi = fmax(0.1f, fmin(M_PI - 0.1f, phi));

        window->camera->position[0] = (radius * sin(phi) * cos(theta)) + window->camera->center[0];
        window->camera->position[1] = (radius * cos(phi)) + window->camera->center[1];
        window->camera->position[2] = (radius * sin(phi) * sin(theta)) + window->camera->center[2];
    }
    // Calculate the forward direction vector
    vec3 forward;
    vec3_sub(forward, window->camera->center, window->camera->eye);
    vec3_norm(forward, forward); // Normalize the forward vector

    // Calculate the right direction vector
    vec3 right;
    vec3_mul_cross(right, forward, window->camera->up);
    vec3_norm(right, right); // Normalize the right vector

    // Movement speed
    float speed = 0.1f;

    // Update camera position based on input
    // if (editor->w_pressed)
    // {
    //     // Move forward
    //     vec3_scale(forward, forward, speed);
    //     vec3_add(window->camera->eye, window->camera->eye, forward);
    //     vec3_add(window->camera->center, window->camera->center, forward);
    // }
    // else if (editor->s_pressed)
    // {
    //     // Move backward
    //     vec3_scale(forward, forward, -speed);
    //     vec3_add(window->camera->eye, window->camera->eye, forward);
    //     vec3_add(window->camera->center, window->camera->center, forward);
    // }
    // else if (editor->a_pressed)
    // {
    //     // Move left
    //     vec3_scale(right, right, -speed);
    //     vec3_add(window->camera->eye, window->camera->eye, right);
    //     vec3_add(window->camera->center, window->camera->center, right);
    // }
    // else if (editor->d_pressed)
    // {
    //     // Move right
    //     vec3_scale(right, right, speed);
    //     vec3_add(window->camera->eye, window->camera->eye, right);
    //     vec3_add(window->camera->center, window->camera->center, right);
    // }

    ACamera->Render(window->camera, window, window->width, window->height, window->scene);

    // AWindowRender->RenderScreen(window->camera);
    AWindowRender->RenderEnd(window);
}

struct AWindow AWindow[1] = {{Init, Destroy, Render}};