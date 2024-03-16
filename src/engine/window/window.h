/**
 * @file window.h
 * @author https://github.com/shaderko
 * @brief This file creates a new window and handles its events.
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <linmath.h>
#include <SDL.h>
#include "../util/util.h"
#include "../render/render.h"
#include "../camera/camera.h"

typedef struct Window Window;
struct Window
{
    int width;
    int height;

    /**
     * @brief SDL window pointer
     */
    SDL_Window *sdl_window;
    SDL_Renderer *renderer;
    SDL_GLContext context;

    WindowRender *render;

    Camera *camera;
};

struct AWindow
{
    /**
     * @brief Initializes a new window
     *
     * @param width Width of the window
     * @param height Height of the window
     * @param title Title of the window
     * @return Window* Pointer to the window
     */
    Window *(*Init)(int width, int height, char *title);

    /**
     * @brief Destroys a window
     */
    void (*Destroy)(Window *window);
};

extern struct AWindow AWindow[1];

#endif