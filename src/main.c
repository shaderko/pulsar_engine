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
    clock_t start, end;
    start = clock();

    // Window *main_window = AWindow->Init(WINDOW_SIZE_W, WINDOW_SIZE_H, "Pulsar Engine Editor");
    Editor *editor = AEditor->Init();

    // Scene *scene = StartCellularAutomaton();
    // editor->scene = scene;

    // All Start functions run here

    end = clock(); // End timer
    double time_taken = ((double)(end - start)) * 1000.0 / CLOCKS_PER_SEC;

    printf("Time to first render: %f ms\n", time_taken);

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
            default:
                break;
            }

            AEditor->UpdateContext(editor, &event);
        }

        ACamera->UpdateView(editor->editor_camera->camera);

        // Run Update from assets

        AEditor->Render(editor);

        // Run LateUpdate from assets
    }

    deleteCellularAutomaton();

    AWindow->Destroy(editor->window);

    puts("Window destroyed, quitting");

    return 0;
}