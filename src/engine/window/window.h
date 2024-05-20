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
#include "../object/map/scene.h"

typedef struct Window Window;
struct Window
{
    // Window size
    int width;
    int height;

    // SDL pointers
    SDL_Window *sdl_window;
    SDL_Renderer *renderer;
    SDL_GLContext context;

    // Camera the window will render from
    Camera *camera;

    // Scene the window will render
    Scene *scene;

    // Window renderer
    WindowRender *render;
};

struct AWindow
{
    /**
     * Initializes a new window
     *
     * @param width Width of the window
     * @param height Height of the window
     * @param title Title of the window
     *
     * @return Window* Pointer to the window
     */
    Window *(*Init)(int width, int height, char *title);

    // Destroys the window
    void (*Destroy)(Window *window);

    // Render the window's contents
    void (*Render)(Window *window);
};

extern struct AWindow AWindow[1];

#endif