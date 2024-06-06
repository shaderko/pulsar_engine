/**
 * @file scene.c
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-05-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>
#include <SDL.h>

#include "../../util/util.h"
#include "scene.h"
#include "../object.h"
#include "../chunk/chunk.h"
#include "../../render/gpu_cache/gpu_cache_manager.h"

static Scene *Init()
{
    Scene *scene = malloc(sizeof(Scene));
    if (!scene)
    {
        ERROR_EXIT("Scene memory couldn't be allocated!\n");
    }

    memset(scene, 0, sizeof(Scene));

    return scene;
}

static void Delete(Scene *scene)
{
    return;
}

static void Update(Scene *scene)
{
    return;
}

// Object is just multiple voxels grouped together, that we can apply gravity forces to
static void AddObject(Scene *scene, Object *object)
{
    return;
}

static void AddChunk(Scene *scene, Chunk *chunk)
{
    scene->chunks = realloc(scene->chunks, sizeof(Chunk *) * (scene->chunks_size + 1));
    scene->chunks[scene->chunks_size] = chunk;
    scene->chunks_size++;
}

// Voxel is the smallest object in the engine, the whole world is made out of voxels
// Voxel has its own position defined with index, which specifies in which position inside a chunk it exists
// Adding a voxel to scene means dynamically changing the size of the chunks defined in a scene so we can add voxels to the specified position
// if there is no chunk, x, y and z coordinates correspond to the world position of chunks

static void AddCamera(Scene *scene, Camera *camera)
{
    if (!scene || !camera)
    {
        return;
    }

    scene->cameras = realloc(scene->cameras, sizeof(Camera *) * (scene->cameras_size + 1));
    if (!scene->cameras)
        ERROR_EXIT("[ERROR] Couldn't allocate memory for scene cameras!\n");

    scene->cameras[scene->cameras_size] = camera;
    scene->cameras_size++;
}

static Chunk *GetChunkAt(Scene *scene, vec3 position)
{
    for (int i = 0; i < scene->chunks_size; ++i)
    {
        if (scene->chunks[i]->position[0] == position[0] && scene->chunks[i]->position[1] == position[1] && scene->chunks[i]->position[2] == position[2])
        {
            return scene->chunks[i];
        }
    }

    return NULL;
}

static void Render(Scene *scene, Camera *camera, int width, int height)
{
    AWindowRender->RayMarchChunkHeightTexture(scene, camera, width, height);

    AGpuCache.Cull();

    // AWindowRender->RenderChunks(scene, camera);
}

// Call write to file function on all chunks
// Save cameras
static void WriteToFile(Scene *scene, const char *file)
{
    // FILE *out = fopen(file, "wb");
    // if (out == NULL)
    // {
    //     printf("Error opening file: %s\n", file);
    //     return;
    // }

    // // Write the total number of objects
    // fwrite(&scene->objects_size, sizeof(int), 1, out);

    // // Write objects data
    // for (int i = 0; i < scene->objects_size; i++)
    // {
    //     Object *object = scene->objects[i];

    //     SerializedDerived serialized_object = AObject.Serialize(object);
    //     fwrite(serialized_object.data, serialized_object.len, 1, out);
    // }

    // fclose(out);
}

static void ReadFile(Scene *scene, const char *file)
{
    // FILE *in = fopen(file, "rb");
    // if (!in)
    // {
    //     printf("Error opening file: %s\n", file);
    //     return;
    // }

    // int size;
    // fread(&size, sizeof(int), 1, in);

    // // Read objects data
    // for (int i = 0; i < size; i++)
    // {
    //     SerializedObject *object = malloc(sizeof(SerializedObject));
    //     fread(object, sizeof(SerializedObject), 1, in);

    //     // Collider
    //     object->collider.derived.data = malloc(object->collider.derived.len);
    //     fread(object->collider.derived.data, object->collider.derived.len, 1, in);

    //     // Renderer
    //     object->renderer.derived.data = malloc(object->renderer.derived.len);
    //     fread(object->renderer.derived.data, object->renderer.derived.len, 1, in);

    //     AScene->Add(scene, AObject.Deserialize(object, scene));

    //     free(object->collider.derived.data);
    //     free(object->renderer.derived.data);
    //     free(object);
    // }
    // fclose(in);
}

struct AScene AScene =
    {
        .Init = Init,
        .Delete = Delete,
        .Update = Update,
        .AddCamera = AddCamera,
        .GetChunkAt = GetChunkAt,
        .AddChunk = AddChunk,
        .Render = Render,
        .WriteToFile = WriteToFile,
        .ReadFile = ReadFile,
};