/**
 * @file editor.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef EDITOR_H
#define EDITOR_H

#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"
#include "../engine/window/window.h"
#include "../engine/object/map/scene.h"

typedef struct Editor Editor;
struct Editor
{
    Window *window;

    ImGuiContext *imgui_context;

    Scene *scene;
};

struct AEditor
{
    Editor *(*Init)();
    void (*Render)(Editor *editor, SDL_Event *event);
};

extern struct AEditor AEditor[1];

#endif