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
#include "../engine/render/gpu_cache/gpu_cache_manager.h"

static Editor *Init()
{
    Editor *editor = malloc(sizeof(Editor));
    if (!editor)
        ERROR_EXIT("Failed to allocate editor struct\n");

    editor->window = AWindow->Init(1280, 720, "Pulsar Engine Editor");
    editor->scene = NULL;

    editor->editor_camera = malloc(sizeof(EditorCamera));
    if (!editor->editor_camera)
    {
        ERROR_EXIT("Failed to allocate editor camera\n");
    }

    memset(editor->editor_camera, 0, sizeof(EditorCamera));
    editor->editor_camera->camera = ACamera->InitPerspective(0.78539816339f, (float)1920 / (float)1080, 0.0001f, 100000.0f);

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
    const char *glsl_version = "#version 430";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return editor;
}

static float radius = 1.0f; // Distance from the center of the cube

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
    // Uint64 start, end;
    // double deltaTime;
    // start = SDL_GetPerformanceCounter();

    ACamera->Render(editor->editor_camera->camera, editor->window, 1920, 1080, editor->scene);

    ImGuiIO *ioptr = igGetIO();

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    igNewFrame();

    // Enable docking in the main viewport
    igDockSpaceOverViewport(NULL, ImGuiDockNodeFlags_None, NULL);

    // end = SDL_GetPerformanceCounter();
    // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
    // printf("Frame Time until ig docker in editor: %f ms\n", deltaTime);

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
                    // editor->scene = AScene->Init((vec3){1, 1, 1});
                }

                // Object *box = AObject.InitBox(false, true, 1, (vec3){0, 0, 0}, (vec3){100, 100, 100});
                // AScene->AddObject(editor->scene, box);
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

                // if (!editor->scene)
                // editor->scene = AScene->Init((vec3){1, 1, 1});
                // else
                // {
                // AScene->Delete(editor->scene);
                // editor->scene = AScene->Init((vec3){1, 1, 1});
                // }
            }
            if (igMenuItem_Bool("Load", NULL, false, true))
            {
                puts("Loading scene...");

                // if (!editor->scene)
                // {
                // puts("Creating new scene...");
                // editor->scene = AScene->Init((vec3){1, 1, 1});
                // }

                // AScene->ReadFile(editor->scene, "scene.bin");
            }
            if (igMenuItem_Bool("Save", NULL, false, true))
            {
                if (!editor->scene)
                {
                    printf("No scene to save!\n");
                    return;
                }
                // AScene->WriteToFile(editor->scene, "scene.bin");
            }
            igEndMenu();
        }
        igEndMainMenuBar();
    }

    {
        igBegin("Performance window", NULL, 0);
        igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / igGetIO()->Framerate, igGetIO()->Framerate);
        if (editor->scene)
        {
            igText("Chunks: %d", editor->scene->chunks_count);
        }

        igEnd();
    }

    {
        igBegin("Gpu cache window", NULL, 0);

        static gpu_cache_manager_t *gpu_cache_manager = NULL;
        if (!gpu_cache_manager)
            gpu_cache_manager = AGpuCache.Get();

        size_t chunk_size = sizeof(gpu_cache_chunk_t *);
        size_t str_size = gpu_cache_manager->chunks_buffer_size * (chunk_size + 1) + 1;
        char *gpu_chunk_cache_str = (char *)malloc(str_size);

        for (size_t i = 0; i < gpu_cache_manager->chunks_buffer_size; i++)
        {
            size_t pos = i * (chunk_size + 1);
            gpu_chunk_cache_str[pos] = '_';
            if (gpu_cache_manager->chunks_buffer[i] && gpu_cache_manager->chunks_buffer[i]->on_gpu)
            {
                for (size_t j = 0; j < chunk_size; j++)
                {
                    gpu_chunk_cache_str[pos + 1 + j] = '|';
                }
            }
            else
            {
                for (size_t j = 0; j < chunk_size; j++)
                {
                    gpu_chunk_cache_str[pos + 1 + j] = '.';
                }
            }
        }
        gpu_chunk_cache_str[str_size - 1] = '\0';

        igTextWrapped("[%s]", gpu_chunk_cache_str);

        free(gpu_chunk_cache_str);

        igEnd();
    }

    if (editor->editor_camera->camera)
    {
        igBegin("Camera window", NULL, 0);

        bool is_pressed = igButton("Create camera", (ImVec2){100, 20});
        if (is_pressed)
        {
            puts("Creating new camera...");
            Camera *camera = ACamera->InitPerspective(0.78539816339f, (float)1920 / (float)1080, 0.0001f, 100000.0f);
            AScene.AddCamera(editor->scene, camera);
        }

        igSliderFloat("UP_X", &editor->editor_camera->camera->up[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("UP_Y", &editor->editor_camera->camera->up[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("UP_Z", &editor->editor_camera->camera->up[2], 0.0f, 100.0f, "%.1f", 0);

        igSliderFloat("CENTER_X", &editor->editor_camera->camera->center[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("CENTER_Y", &editor->editor_camera->camera->center[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("CENTER_Z", &editor->editor_camera->camera->center[2], 0.0f, 100.0f, "%.1f", 0);

        igSliderFloat("POSITION_X", &editor->editor_camera->camera->position[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("POSITION_Y", &editor->editor_camera->camera->position[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("POSITION_Z", &editor->editor_camera->camera->position[2], 0.0f, 100.0f, "%.1f", 0);

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

            editor->editor_camera->camera->position[0] = (radius * sin(phi) * cos(theta)) + editor->editor_camera->camera->center[0];
            editor->editor_camera->camera->position[1] = (radius * cos(phi)) + editor->editor_camera->camera->center[1];
            editor->editor_camera->camera->position[2] = (radius * sin(phi) * sin(theta)) + editor->editor_camera->camera->center[2];
        }
        // Calculate the forward direction vector
        vec3 forward;
        vec3_sub(forward, editor->editor_camera->camera->center, editor->editor_camera->camera->eye);
        vec3_norm(forward, forward); // Normalize the forward vector

        // Calculate the right direction vector
        vec3 right;
        vec3_mul_cross(right, forward, editor->editor_camera->camera->up);
        vec3_norm(right, right); // Normalize the right vector

        // Movement speed
        float speed = 1.0f;

        // Update camera position based on input
        if (editor->w_pressed)
        {
            // Move forward
            vec3_scale(forward, forward, speed);
            vec3_add(editor->editor_camera->camera->eye, editor->editor_camera->camera->eye, forward);
            vec3_add(editor->editor_camera->camera->center, editor->editor_camera->camera->center, forward);
        }
        else if (editor->s_pressed)
        {
            // Move backward
            vec3_scale(forward, forward, -speed);
            vec3_add(editor->editor_camera->camera->eye, editor->editor_camera->camera->eye, forward);
            vec3_add(editor->editor_camera->camera->center, editor->editor_camera->camera->center, forward);
        }
        else if (editor->a_pressed)
        {
            // Move left
            vec3_scale(right, right, -speed);
            vec3_add(editor->editor_camera->camera->eye, editor->editor_camera->camera->eye, right);
            vec3_add(editor->editor_camera->camera->center, editor->editor_camera->camera->center, right);
        }
        else if (editor->d_pressed)
        {
            // Move right
            vec3_scale(right, right, speed);
            vec3_add(editor->editor_camera->camera->eye, editor->editor_camera->camera->eye, right);
            vec3_add(editor->editor_camera->camera->center, editor->editor_camera->camera->center, right);
        }
        // For simplicity, the code for up/down movement has been omitted, but would involve adjusting
        // the camera's eye and center positions along the world's up axis (usually (0, 1, 0) or similar).
    }

    // end = SDL_GetPerformanceCounter();
    // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
    // printf("Frame Time until camera setup in editor: %f ms\n", deltaTime);

    if (editor->editor_camera->camera)
    {
        igBegin("Editor Camera Render", NULL, 0);

        ImVec2 windowSize;
        igGetContentRegionAvail(&windowSize);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            fprintf(stderr, "OpenGL Camera Error: %d\n", error);
        }

        // end = SDL_GetPerformanceCounter();
        // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
        // printf("Frame Time after camera render but before show image in editor: %f ms\n", deltaTime);

        ImTextureID myTextureID = (ImTextureID)editor->editor_camera->camera->image_out; // Cast your texture identifier to ImTextureID
        ImVec2 imageSize = (ImVec2){windowSize.x, windowSize.y};                         // Display the image as 100x100 pixels
        ImVec2 uv0 = (ImVec2){0, 0};                                                     // Use the whole texture
        ImVec2 uv1 = (ImVec2){1, 1};
        ImVec4 tintCol = (ImVec4){1.0f, 1.0f, 1.0f, 1.0f};
        ImVec4 borderCol = (ImVec4){0.0f, 0.0f, 0.0f, 0.0f};

        igImage(myTextureID, imageSize, uv0, uv1, tintCol, borderCol);

        // end = SDL_GetPerformanceCounter();
        // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
        // printf("Frame Time after show image render in editor: %f ms\n", deltaTime);

        igEnd();
    }

    // end = SDL_GetPerformanceCounter();
    // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
    // printf("Frame Time after camera render in editor: %f ms\n", deltaTime);

    for (int i = 0; i < editor->scene->cameras_size; i++)
    {
        Camera *camera = editor->scene->cameras[i];

        igSliderFloat("UP_X", &camera->up[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("UP_Y", &camera->up[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("UP_Z", &camera->up[2], 0.0f, 100.0f, "%.1f", 0);

        igSliderFloat("CENTER_X", &camera->center[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("CENTER_Y", &camera->center[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("CENTER_Z", &camera->center[2], 0.0f, 100.0f, "%.1f", 0);

        igSliderFloat("POSITION_X", &camera->position[0], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("POSITION_Y", &camera->position[1], 0.0f, 100.0f, "%.1f", 0);
        igSliderFloat("POSITION_Z", &camera->position[2], 0.0f, 100.0f, "%.1f", 0);

        char windowTitle[256];
        snprintf(windowTitle, sizeof(windowTitle), "Camera Render #%d", i);

        igBegin(&windowTitle, NULL, 0);

        ImVec2 windowSize;
        igGetContentRegionAvail(&windowSize);

        ACamera->UpdateView(camera);
        ACamera->Render(camera, editor->window, windowSize.x, windowSize.y, editor->scene);

        ImTextureID myTextureID = (ImTextureID)camera->image_out; // Cast your texture identifier to ImTextureID
        ImVec2 imageSize = (ImVec2){windowSize.x, windowSize.y};  // Display the image as 100x100 pixels
        ImVec2 uv0 = (ImVec2){0, 0};                              // Use the whole texture
        ImVec2 uv1 = (ImVec2){1, 1};
        ImVec4 tintCol = (ImVec4){1.0f, 1.0f, 1.0f, 1.0f};
        ImVec4 borderCol = (ImVec4){0.0f, 0.0f, 0.0f, 0.0f};

        igImage(myTextureID, imageSize, uv0, uv1, tintCol, borderCol);

        igEnd();
    }

    // end = SDL_GetPerformanceCounter();
    // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
    // printf("Frame Time until ig render in editor: %f ms\n", deltaTime);

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

    // end = SDL_GetPerformanceCounter();
    // deltaTime = (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();
    // printf("Frame Time until swap window in editor: %f ms\n", deltaTime);
}

struct AEditor AEditor[1] = {{Init, UpdateContext, Render}};