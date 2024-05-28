#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <time.h>
#include <stdlib.h>

#include "engine/window/window.h"
#include "engine/object/object.h"
#include "engine/camera/camera.h"
#include "editor/editor.h"
#include "engine/object/chunk/chunk.h"
#include "engine/object/chunk/octree/octree.h"

#include "../assets/cellular_automaton.h"

#define WINDOW_SIZE_W 1920
#define WINDOW_SIZE_H 1080

#define TARGET_FPS 60
#define FRAME_DURATION (1000 / TARGET_FPS)

int main(int argc, char *argv[])
{
    srand(time(NULL));

    clock_t start, end;
    start = clock();

    Editor *editor = AEditor->Init();
    Scene *scene = AScene.Init();

    editor->scene = scene;

    for (int xx = 0; xx < 2; xx++)
    {
        for (int yy = 0; yy < 2; yy++)
        {
            for (int zz = 0; zz < 2; zz++)
            {
                Chunk *chunk = AChunk.Init((vec3){xx, yy, zz});
                AScene.AddChunk(scene, chunk);

                for (int x = 0; x < 15; x++)
                {
                    for (int y = 0; y < 15; y++)
                    {
                        for (int z = 0; z < 15; z++)
                        {
                            AChunk.Add(chunk, x, y, z, 0, 0);
                        }
                    }
                }
            }
        }
    }

    // All Start functions run here

    end = clock(); // End timer
    double time_taken = ((double)(end - start)) * 1000.0 / CLOCKS_PER_SEC;
    printf("[INFO] Time to first render: %f ms\n", time_taken);

    bool quit = false;
    while (!quit)
    {
        Uint32 frameStart = SDL_GetTicks(); // Get the start time of the frame

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

        // Measure the frame duration and delay if necessary
        // Uint32 frameTime = SDL_GetTicks() - frameStart;
        // if (frameTime < FRAME_DURATION)
        // {
        //     SDL_Delay(FRAME_DURATION - frameTime);
        // }
    }

    deleteCellularAutomaton();

    AWindow->Destroy(editor->window);

    puts("Window destroyed, quitting");

    return 0;
}