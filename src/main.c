#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "engine/window/window.h"
#include "engine/object/object.h"
#include "engine/camera/camera.h"
#include "editor/editor.h"

#include "../assets/cellular_automaton.h"

#define WINDOW_SIZE_W 1280
#define WINDOW_SIZE_H 720

int main(int argc, char *argv[])
{

    // Window *main_window = AWindow->Init(WINDOW_SIZE_W, WINDOW_SIZE_H, "Pulsar Engine Editor");
    Editor *editor = AEditor->Init();
    // editor->window->camera = ACamera->InitOrtho(-17.777779, 17.777779, -10, 10, 0.1f, 100.0f);
    editor->window->camera = ACamera->InitPerspective(0.78539816339f, (float)1920 / (float)1080, 0.0001f, 100000.0f);

    // Camera *camera = ACamera->Init(100, 10, (vec3){0, 0, 0}, NULL);
    // main_window->camera = camera;

    // Object *cube = AObject.InitBox(true, true, .1f, (vec3){0, 0, 0}, (vec3){10, 10, 10});

    // Scene *scene = StartCellularAutomaton();
    // editor->scene = scene;

    bool quit = false;
    while (!quit)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {

            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                puts("QUIT");
                break;
            // case SDL_MOUSEWHEEL:
            //     if (event.wheel.y > 0)
            //     {
            //         printf("mouse wheel up\n");
            //         // camera->distance -= 10;
            //     }
            //     else if (event.wheel.y < 0)
            //     {
            //         printf("mouse wheel down\n");
            //         // camera->distance += 10;
            //     }
            // case SDL_KEYDOWN:
            //     if (event.key.keysym.sym == SDLK_a)
            //     {
            //         printf("a\n");
            //         // camera->position[0] -= 10;
            //     }
            //     else if (event.key.keysym.sym == SDLK_d)
            //     {
            //         printf("d\n");
            //         // camera->position[0] += 10;
            //     }
            //     else if (event.key.keysym.sym == SDLK_w)
            //     {
            //         printf("w\n");
            //         // camera->position[1] += 10;
            //     }
            //     else if (event.key.keysym.sym == SDLK_s)
            //     {
            //         printf("s\n");
            //         // camera->position[1] -= 10;
            //     }
            default:
                break;
            }

            AEditor->UpdateContext(editor, &event);
        }

        ACamera->UpdateView(editor->window->camera);

        // Run main from assets

        AEditor->Render(editor);
    }

    // deleteCellularAutomaton();

    AWindow->Destroy(editor->window);

    puts("Window destroyed, quitting");

    return 0;
}