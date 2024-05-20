#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "engine/window/window.h"
#include "engine/object/object.h"
#include "engine/camera/camera.h"
#include "editor/editor.h"
#include "engine/object/chunk/chunk.h"
#include "engine/object/chunk/octree/octree.h"

#include "../assets/cellular_automaton.h"

#define WINDOW_SIZE_W 1280
#define WINDOW_SIZE_H 720

int main(int argc, char *argv[])
{
    clock_t start, end;
    start = clock();

    // Window *main_window = AWindow->Init(WINDOW_SIZE_W, WINDOW_SIZE_H, "Pulsar Engine Editor");
    // Camera *main_camera = ACamera->InitPerspective(0.78539816339f, (float)WINDOW_SIZE_W / (float)WINDOW_SIZE_H, 0.0001f, 100000.0f);

    Editor *editor = AEditor->Init();
    // main_window->camera = main_camera;

    // Scene *scene = StartCellularAutomaton();
    Scene *scene = AScene.Init();
    // main_window->scene = scene;
    editor->scene = scene;

    Chunk *chunk = AChunk.Init((vec3){0, 1, 0});
    AScene.AddChunk(scene, chunk);
    for (int x = 0; x < 5; x++)
    {
        for (int y = 0; y < 5; y++)
        {
            for (int z = 0; z < 5; z++)
            {
                AChunk.Add(chunk, x, y, z, 0, 0);
            }
        }
    }

    // Chunk *chunk = AChunk.Init((vec3){0, 1, 0});
    // AScene.AddChunk(scene, chunk);
    // // AChunk.Add(chunk, 45, 12, 19, 0, 0);
    // // AChunk.Add(chunk, 45, 13, 18, 0, 0);
    // // AChunk.Add(chunk, 46, 13, 18, 0, 0);
    // // AChunk.Add(chunk, 45, 13, 19, 0, 0);
    // // AChunk.Add(chunk, 46, 11, 19, 0, 0);
    // // AChunk.Add(chunk, 44, 12, 19, 0, 0);
    // // AChunk.Add(chunk, 45, 12, 19, 0, 0);
    // // AChunk.Add(chunk, 32, 32, 32, 0, 0);
    // // AChunk.Add(chunk, 12, 12, 12, 0, 0);
    // AChunk.Add(chunk, 63, 63, 63, 0, 0);
    // AChunk.Add(chunk, 62, 62, 62, 0, 0);
    // AChunk.Add(chunk, 61, 61, 61, 0, 0);
    // AChunk.Add(chunk, 60, 60, 60, 0, 0);
    // AChunk.Add(chunk, 59, 59, 59, 0, 0);
    // AChunk.Add(chunk, 58, 58, 58, 0, 0);
    // AChunk.Add(chunk, 57, 57, 57, 0, 0);
    // AChunk.Add(chunk, 56, 56, 56, 0, 0);
    // AChunk.Add(chunk, 55, 55, 55, 0, 0);
    // AChunk.Add(chunk, 54, 54, 54, 0, 0);
    // AChunk.Add(chunk, 53, 53, 53, 0, 0);
    // AChunk.Add(chunk, 52, 52, 52, 0, 0);
    // AChunk.Add(chunk, 51, 51, 51, 0, 0);
    // AChunk.Add(chunk, 0, 0, 0, 0, 0);
    // AChunk.Add(chunk, 0, 0, 63, 0, 0);
    // AChunk.Add(chunk, 0, 63, 0, 0, 0);
    // AChunk.Add(chunk, 0, 63, 63, 0, 0);
    // AChunk.Add(chunk, 29, 29, 29, 0, 0);
    // AChunk.Add(chunk, 63, 63, 63, 0, 0);
    // AChunk.Add(chunk, 63, 0, 0, 0, 0);
    // AChunk.Add(chunk, 63, 63, 0, 0, 0);
    // AChunk.Add(chunk, 63, 0, 63, 0, 0);

    // Chunk *chunk2 = AChunk.Init((vec3){2, 0, 0});
    // AChunk.Add(chunk2, 0, 0, 0, 0, 0);
    // // AChunk.Add(chunk2, 0, 0, 45, 0, 0);
    // // AChunk.Add(chunk2, 0, 45, 45, 0, 0);
    // // AChunk.Add(chunk2, 0, 45, 0, 0, 0);
    // AChunk.Add(chunk2, 45, 45, 45, 0, 0);
    // AChunk.Add(chunk2, 45, 45, 0, 0, 0);
    // AChunk.Add(chunk2, 45, 0, 0, 0, 0);
    // AChunk.Add(chunk2, 45, 0, 45, 0, 0);
    // AScene.AddChunk(scene, chunk2);

    // Chunk *chunk3 = AChunk.Init((vec3){64, 64, 64});
    // AScene.AddChunk(scene, chunk3);

    // AOctree.VisualizeOctree(chunk->voxel_tree);

    // return 0;

    // All Start functions run here

    end = clock(); // End timer
    double time_taken = ((double)(end - start)) * 1000.0 / CLOCKS_PER_SEC;

    printf("Time to first render: %f ms\n", time_taken);

    bool quit = false;
    while (!quit)
    {
        // Uint64 start, end;
        // double deltaTime;
        // start = SDL_GetPerformanceCounter();

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
        // ACamera->UpdateView(main_camera);
        // end = SDL_GetPerformanceCounter();
        // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
        // printf("Frame Time until update view In main: %f ms\n", deltaTime);

        // Run Update from assets

        // AWindow->Render(main_window);

        AEditor->Render(editor);

        // Run LateUpdate from assets

        // quit = true;
        // end = SDL_GetPerformanceCounter();
        // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
        // printf("Frame Time In main: %f ms\n", deltaTime);

        // break;
    }

    deleteCellularAutomaton();

    AWindow->Destroy(editor->window);

    puts("Window destroyed, quitting");

    return 0;
}