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
#include "window.h"

static Window *Init(int width, int height, char *title)
{
    Window *window = malloc(sizeof(Window));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        ERROR_EXIT("couldn't initialize SDL: %s\n", SDL_GetError());

// Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // and prepare OpenGL stuff
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
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

    puts("openGL loaded");
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

struct AWindow AWindow[1] = {{Init, Destroy}};