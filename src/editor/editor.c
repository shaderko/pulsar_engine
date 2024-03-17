/**
 * @file editor.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "editor.h"
#include <SDL.h>
#include "../engine/util/util.h"

static Editor *Init()
{
    Editor *editor = malloc(sizeof(Editor));
    if (!editor)
        ERROR_EXIT("Failed to allocate editor struct\n");

    editor->window = AWindow->Init(1280, 720, "Pulsar Engine Editor");
    editor->scene = NULL;

    SDL_GL_SetSwapInterval(1); // enable vsync

    // setup imgui
    editor->imgui_context = igCreateContext(NULL);
    igSetCurrentContext(editor->imgui_context);

    // set docking
    ImGuiIO *ioptr = igGetIO();
    ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
#ifdef IMGUI_HAS_DOCK
    ioptr->ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    ioptr->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
#endif

    igStyleColorsDark(NULL);

    ImGui_ImplSDL2_InitForOpenGL(editor->window->sdl_window, editor->window->context);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return editor;
}

static void Render(Editor *editor, SDL_Event *event)
{
    SDL_GL_MakeCurrent(editor->window->sdl_window, editor->window->context);
    igSetCurrentContext(editor->imgui_context);

    if (event)
        ImGui_ImplSDL2_ProcessEvent(event);

    ImGuiIO *ioptr = igGetIO();

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    igNewFrame();

    // Enable docking in the main viewport
    igDockSpaceOverViewport(NULL, ImGuiDockNodeFlags_None, NULL);

    if (igBeginMainMenuBar())
    {
        if (igBeginMenu("Create", true))
        {
            if (igMenuItem_Bool("Box", NULL, false, true))
            {
                puts("Creating Box...");

                if (!editor->scene)
                {
                    puts("Creating new scene...");
                    editor->scene = AScene->Init((vec3){1, 1, 1});
                }

                Object *box = AObject.InitBox(false, true, 1, (vec3){0, 0, 0}, (vec3){100, 100, 100});
                AScene->Add(editor->scene, box);
            }
            if (igMenuItem_Bool("Sphere", NULL, false, true))
            {
                // Code to create a sphere...
            }
            if (igMenuItem_Bool("Light", NULL, false, true))
            {
                // Code to create a light...
            }
            igEndMenu();
        }
        if (igBeginMenu("Scene", true))
        {
            if (igMenuItem_Bool("Load", NULL, false, true))
            {
                puts("Loading scene...");

                if (!editor->scene)
                {
                    puts("Creating new scene...");
                    editor->scene = AScene->Init((vec3){1, 1, 1});
                }

                AScene->ReadFile(editor->scene, "scene.bin");
            }
            if (igMenuItem_Bool("Save", NULL, false, true))
            {
                if (!editor->scene)
                {
                    printf("No scene to save!\n");
                    return;
                }
                AScene->WriteToFile(editor->scene, "scene.bin");
            }
            igEndMenu();
        }
        igEndMainMenuBar();
    }

    // show a simple window that we created ourselves.
    {
        float f = 0.0f;
        int counter = 0;

        igBegin("Performance window", NULL, 0);
        igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        igSameLine(0.0f, -1.0f);
        igText("counter = %d", counter);

        igEnd();
    }

    // {
    //     // camera window
    //     igBegin("Camera", NULL, 0);
    //     // igSliderFloat("FOV", &editor->camera.fov, 10.0f, 170.0f, "%.1f", NULL);
    // }

    // render
    igRender();
    glViewport(0, 0, (int)ioptr->DisplaySize.x, (int)ioptr->DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

#ifdef IMGUI_HAS_DOCK
    if (ioptr->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        igUpdatePlatformWindows();
        igRenderPlatformWindowsDefault(NULL, NULL);
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
#endif

    SDL_GL_SwapWindow(editor->window->sdl_window);
}

struct AEditor AEditor[1] = {{Init, Render}};