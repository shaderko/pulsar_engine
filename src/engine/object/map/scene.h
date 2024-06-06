/**
 * @file scene.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-05-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SCENE_H
#define SCENE_H

#include <linmath.h>

#include "../../camera/camera.h"
#include "../chunk/chunk.h"

typedef struct Scene Scene;
struct Scene
{
    Camera **cameras;
    size_t cameras_size;

    /**
     * Array of chunks in a scene
     */
    Chunk **chunks;
    size_t chunks_size;

    size_t chunks_count;
};

struct AScene
{
    /**
     * Initializes a scene
     */
    Scene *(*Init)();
    void (*Delete)(Scene *scene);

    /**
     * Updates a scene
     */
    void (*Update)(Scene *scene);

    /**
     * Adds an object to a scene
     */
    // void (*AddObject)(Scene *scene, Object *object);

    void (*AddCamera)(Scene *scene, Camera *camera);

    Chunk *(*GetChunkAt)(Scene *scene, vec3 position);

    void (*AddChunk)(Scene *scene, Chunk *chunk);

    void (*Render)(Scene *scene, Camera *camera, int width, int height);

    /**
     * Writes scene objects to a file
     */
    void (*WriteToFile)(Scene *scene, const char *file);

    /**
     * Reads scene objects from a file
     */
    void (*ReadFile)(Scene *scene, const char *file);
};

extern struct AScene AScene;

#endif