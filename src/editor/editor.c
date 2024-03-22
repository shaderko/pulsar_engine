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
#include <linmath.h>
#include "../engine/util/util.h"
#include "../engine/object/object.h"

static Editor *Init()
{
    Editor *editor = malloc(sizeof(Editor));
    if (!editor)
        ERROR_EXIT("Failed to allocate editor struct\n");

    editor->window = AWindow->Init(1280, 720, "Pulsar Engine Editor");
    editor->scene = NULL;

    // TODO: delete, only for testing
    editor->btn_pressed = false;
    memcpy(editor->velocity, (vec3){0, 0, 0}, sizeof(vec3));
    editor->a_pressed = false;
    editor->d_pressed = false;
    editor->w_pressed = false;
    editor->s_pressed = false;
    editor->space_pressed = false;
    editor->shift_pressed = false;

    // SDL_GL_SetSwapInterval(1); // enable vsync

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
    const char *glsl_version = "#version 410";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return editor;
}

static float radius = 50.0f; // Distance from the center of the cube

static void UpdateContext(Editor *editor, SDL_Event *event)
{
    igSetCurrentContext(editor->imgui_context);

    if (event)
        ImGui_ImplSDL2_ProcessEvent(event);

    switch (event->type)
    {
    case SDL_MOUSEWHEEL:
        if (event->wheel.y > 0)
        {
            radius -= 10.0f;
        }
        else if (event->wheel.y < 0)
        {
            radius += 10.0f;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            puts("Middle mouse button pressed");
            editor->btn_pressed = true;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            puts("Middle mouse button released");
            editor->btn_pressed = false;
        }
        break;
    case SDL_KEYDOWN:
        if (event->key.keysym.sym == SDLK_a)
        {
            editor->a_pressed = true;
        }
        else if (event->key.keysym.sym == SDLK_d)
        {
            editor->d_pressed = true;
        }
        else if (event->key.keysym.sym == SDLK_w)
        {
            editor->w_pressed = true;
        }
        else if (event->key.keysym.sym == SDLK_s)
        {
            editor->s_pressed = true;
        }
        else if (event->key.keysym.sym == SDLK_SPACE)
        {
            editor->space_pressed = true;
        }
        else if (event->key.keysym.sym == SDLK_LSHIFT)
        {
            editor->shift_pressed = true;
        }
        break;
    case SDL_KEYUP:
        if (event->key.keysym.sym == SDLK_a)
        {
            editor->a_pressed = false;
        }
        else if (event->key.keysym.sym == SDLK_d)
        {
            editor->d_pressed = false;
        }
        else if (event->key.keysym.sym == SDLK_w)
        {
            editor->w_pressed = false;
        }
        else if (event->key.keysym.sym == SDLK_s)
        {
            editor->s_pressed = false;
        }
        else if (event->key.keysym.sym == SDLK_SPACE)
        {
            editor->space_pressed = false;
        }
        else if (event->key.keysym.sym == SDLK_LSHIFT)
        {
            editor->shift_pressed = false;
        }
        break;
    default:
        break;
    }
}

static void Render(Editor *editor)
{
    SDL_GL_MakeCurrent(editor->window->sdl_window, editor->window->context);

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
            if (igMenuItem_Bool("New", NULL, false, true))
            {
                puts("Creating new scene...");

                if (!editor->scene)
                    editor->scene = AScene->Init((vec3){1, 1, 1});
                else
                {
                    AScene->Delete(editor->scene);
                    editor->scene = AScene->Init((vec3){1, 1, 1});
                }
            }
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

    {
        igBegin("Performance window", NULL, 0);
        igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);

        // char input[256] = "";
        // igInputText("Text", &input, 256, ImGuiInputTextFlags_EnterReturnsTrue, NULL, NULL);

        bool createSingleObject = igButton("Create 1 object", (ImVec2){200, 20});
        if (createSingleObject)
        {
            if (!editor->scene)
            {
                puts("Creating new scene...");
                editor->scene = AScene->Init((vec3){1, 1, 1});
            }

            static int pos = 0;

            static Model *model1;
            if (!model1)
                model1 = AModel->Load("assets/untitled.obj");

            // Object *box = AObject.InitMesh((vec3){1, 1, 1}, (vec3){1, 1, 1}, (vec3){1, 1, 1}, model);
            Object *box = AObject.InitMesh((vec3){pos * 20, 0, 0}, (vec3){0, 0, 0}, (vec3){10, 10, 10}, model1);
            pos += 1;
            AScene->Add(editor->scene, box);
        }

        bool createFiveObjects = igButton("Create 5 objects", (ImVec2){200, 20});
        if (createFiveObjects)
        {
            if (!editor->scene)
            {
                puts("Creating new scene...");
                editor->scene = AScene->Init((vec3){1, 1, 1});
            }

            for (int k = 0; k < 5; k++)
            {
                Model *model2 = AModel->Load("assets/bunny.obj");

                Object *box = AObject.InitMesh((vec3){k * 1, 1, 1}, (vec3){0, 0, 0}, (vec3){10, 10, 10}, model2);
                AScene->Add(editor->scene, box);
            }
        }

        bool createLowObjects = igButton("Create 125 objects", (ImVec2){200, 20});
        if (createLowObjects)
        {
            if (!editor->scene)
            {
                puts("Creating new scene...");
                editor->scene = AScene->Init((vec3){1, 1, 1});
            }

            for (int i = 0; i < 5; i++)
            {
                for (int j = 0; j < 5; j++)
                {
                    for (int k = 0; k < 5; k++)
                    {
                        static Model *model;
                        if (!model)
                            model = AModel->Load("assets/bunny.obj");

                        Object *box = AObject.InitMesh((vec3){k * 10, j * 10, i * 10}, (vec3){1, 1, 1}, (vec3){10, 10, 10}, model);
                        AScene->Add(editor->scene, box);
                    }
                }
            }
        }

        bool createObjects = igButton("Create 15625 objects", (ImVec2){200, 20});
        if (createObjects)
        {
            if (!editor->scene)
            {
                puts("Creating new scene...");
                editor->scene = AScene->Init((vec3){1, 1, 1});
            }

            for (int i = 0; i < 25; i++)
            {
                for (int j = 0; j < 25; j++)
                {
                    for (int k = 0; k < 25; k++)
                    {
                        Object *box = AObject.InitBox((vec3){j * 10, i * 10, k * 10}, (vec3){0, 0, 0}, (vec3){10, 10, 10});
                        AScene->Add(editor->scene, box);
                    }
                }
            }
        }
        if (editor->scene)
        {
            int objects_count = 0;
            for (int i = 0; i < editor->scene->objects_list_size; i++)
            {
                objects_count += editor->scene->objects_list[i]->object_size;
            }

            igText("Objects: %d", objects_count);
            // int indicies = 0;
            // for (int i = 0; i < editor->scene->objects_size; i++)
            // {
            //     indicies += editor->scene->objects[i]->renderer->model->indicies_count;
            // }
            // igText("Indicies: %d", indicies);
            // igText("Triangles: %d", indicies / 3);
            // int verticies = 0;
            // for (int i = 0; i < editor->scene->objects_size; i++)
            // {
            //     verticies += editor->scene->objects[i]->renderer->model->verticies_count;
            // }
            // igText("Verticies: %d", verticies);
        }

        igEnd();
    }

    if (editor->window->camera)
    {
        igBegin("Camera window", NULL, 0);

        bool is_pressed = igButton("Create camera", (ImVec2){100, 20});
        if (is_pressed)
        {
            free(editor->window->camera);

            editor->window->camera = ACamera->InitPerspective(0.78539816339f, (float)1920 / (float)1080, 0.0001f, 100000.0f);
        }

        igSliderFloat("UP_X", &editor->window->camera->up[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("UP_Y", &editor->window->camera->up[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("UP_Z", &editor->window->camera->up[2], 0.0f, 100.0f, "%.1f", 0);

        igSliderFloat("CENTER_X", &editor->window->camera->center[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("CENTER_Y", &editor->window->camera->center[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("CENTER_Z", &editor->window->camera->center[2], 0.0f, 100.0f, "%.1f", 0);

        igSliderFloat("POSITION_X", &editor->window->camera->position[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("POSITION_Y", &editor->window->camera->position[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("POSITION_Z", &editor->window->camera->position[2], 0.0f, 100.0f, "%.1f", 0);

        igEnd();

        static int lastX = 0, lastY = 0; // Static variables to remember the last position
        int x, y;
        SDL_GetMouseState(&x, &y);
        if (editor->btn_pressed)
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

            editor->window->camera->position[0] = (radius * sin(phi) * cos(theta)) + editor->window->camera->center[0];
            editor->window->camera->position[1] = (radius * cos(phi)) + editor->window->camera->center[1];
            editor->window->camera->position[2] = (radius * sin(phi) * sin(theta)) + editor->window->camera->center[2];
        }
        // Calculate the forward direction vector
        vec3 forward;
        vec3_sub(forward, editor->window->camera->center, editor->window->camera->eye);
        vec3_norm(forward, forward); // Normalize the forward vector

        // Calculate the right direction vector
        vec3 right;
        vec3_mul_cross(right, forward, editor->window->camera->up);
        vec3_norm(right, right); // Normalize the right vector

        // Movement speed
        float speed = 0.1f;

        // Update camera position based on input
        if (editor->w_pressed)
        {
            // Move forward
            vec3_scale(forward, forward, speed);
            vec3_add(editor->window->camera->eye, editor->window->camera->eye, forward);
            vec3_add(editor->window->camera->center, editor->window->camera->center, forward);
        }
        else if (editor->s_pressed)
        {
            // Move backward
            vec3_scale(forward, forward, -speed);
            vec3_add(editor->window->camera->eye, editor->window->camera->eye, forward);
            vec3_add(editor->window->camera->center, editor->window->camera->center, forward);
        }
        else if (editor->a_pressed)
        {
            // Move left
            vec3_scale(right, right, -speed);
            vec3_add(editor->window->camera->eye, editor->window->camera->eye, right);
            vec3_add(editor->window->camera->center, editor->window->camera->center, right);
        }
        else if (editor->d_pressed)
        {
            // Move right
            vec3_scale(right, right, speed);
            vec3_add(editor->window->camera->eye, editor->window->camera->eye, right);
            vec3_add(editor->window->camera->center, editor->window->camera->center, right);
        }
        // For simplicity, the code for up/down movement has been omitted, but would involve adjusting
        // the camera's eye and center positions along the world's up axis (usually (0, 1, 0) or similar).
    }

    if (editor->window->camera)
    {
        igBegin("Camera Render", NULL, 0);

        ImVec2 windowSize;
        igGetContentRegionAvail(&windowSize);

        ACamera->Render(editor->window->camera, editor->window, windowSize.x, windowSize.y, editor->scene);

        ImTextureID myTextureID = (ImTextureID)editor->window->camera->color; // Cast your texture identifier to ImTextureID
        ImVec2 imageSize = (ImVec2){windowSize.x, windowSize.y};              // Display the image as 100x100 pixels
        ImVec2 uv0 = (ImVec2){0, 0};                                          // Use the whole texture
        ImVec2 uv1 = (ImVec2){1, 1};
        ImVec4 tintCol = (ImVec4){1.0f, 1.0f, 1.0f, 1.0f};
        ImVec4 borderCol = (ImVec4){0.0f, 0.0f, 0.0f, 0.0f};

        igImage(myTextureID, imageSize, uv0, uv1, tintCol, borderCol);

        igEnd();
    }

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

struct AEditor AEditor[1] = {{Init, UpdateContext, Render}};