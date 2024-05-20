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

#include "../../util/util.h"
#include "scene.h"
#include "../object.h"
#include "../chunk/chunk.h"
#include <SDL.h>

#define MAX_WORLD_X_SIZE 12
#define MAX_WORLD_Y_SIZE 12
#define MAX_WORLD_Z_SIZE 12
#define MAX_WORLD_SIZE MAX_WORLD_X_SIZE *MAX_WORLD_Y_SIZE *MAX_WORLD_Z_SIZE

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

static void Render(Scene *scene, Camera *camera, int width, int height)
{
    AWindowRender->RenderSceneChunks(scene, camera, width, height);
}

static int SerializeThreadFunc(void *data)
{
    AChunk.Serialize(data);
    return 0;
}

static SerializedScene SerializeChunks(Scene *scene)
{
    // If chunk size is 0, we don't need to serialize anything
    if (!scene || scene->chunks_size == 0)
        return (SerializedScene){0};

    // Create all needed buffers for storage and threads
    SDL_Thread **threads = malloc(scene->chunks_size * sizeof(SDL_Thread *));
    GPUChunk *gpu_chunks = malloc(MAX_WORLD_SIZE * sizeof(GPUChunk));
    GPUChunk default_chunk = {0, 0, 0, (unsigned int)false};
    for (int i = 0; i < MAX_WORLD_SIZE; ++i)
    {
        gpu_chunks[i] = default_chunk;
    }
    SerializedChunk *serialized_chunks = malloc(scene->chunks_size * sizeof(SerializedChunk));

    if (!threads || !serialized_chunks || !gpu_chunks)
        ERROR_EXIT("Failed to allocate memory for scene serialization!\n");

    // Create threads to process each chunk
    for (int i = 0; i < scene->chunks_size; i++)
    {
        struct ThreadData
        {
            Chunk *chunk;
            SerializedChunk *result;
        } *data = malloc(sizeof(struct ThreadData));

        data->chunk = scene->chunks[i];
        data->result = &serialized_chunks[i];

        threads[i] = SDL_CreateThread(SerializeThreadFunc, "Chunk Serialize", data);
        if (!threads[i])
            ERROR_EXIT("Failed to create chunk serialize thread!\n");

        // puts("[INFO] Created a thread.");
    }

    // puts("[INFO] Waiting for all threads to finish");

    // Wait for all threads to finish and calculate total size
    unsigned int totalSize = 0;
    for (int i = 0; i < scene->chunks_size; i++)
    {
        int threadResult = 0;
        SDL_WaitThread(threads[i], &threadResult);

        // Create the gpu chunk
        unsigned int x = (scene->chunks[i]->position >> 20) & 0x3FF;
        unsigned int y = (scene->chunks[i]->position >> 10) & 0x3FF;
        unsigned int z = scene->chunks[i]->position & 0x3FF;
        unsigned int index = x + y * 12 + z * 12 * 12;
        gpu_chunks[index] = (GPUChunk){scene->chunks[i]->position, totalSize, serialized_chunks[i].size, (unsigned int)true};

        // Add the size to the overall size
        totalSize += serialized_chunks[i].size;
    }

    // printf("[INFO] Combining results, combined size %i, in bytes %i\n", totalSize, totalSize * sizeof(unsigned int));

    // Combine all results into a single buffer
    unsigned int *combined_data = malloc(totalSize * sizeof(unsigned int));
    unsigned int *current_position = combined_data;

    for (int i = 0; i < scene->chunks_size; i++)
    {
        memcpy(current_position, serialized_chunks[i].data, serialized_chunks[i].size * sizeof(unsigned int));
        // for (int j = 0; j < serialized_chunks[i].size; j++)
        // {
        //     AOctree.print_binary(combined_data[j]);
        //     printf("\n");
        // }
        // printf("\n");
        current_position += serialized_chunks[i].size;
        free(serialized_chunks[i].data); // Free each chunk's data after copying
    }

    // puts("[DEBUG] Combining successful");

    free(threads);
    free(serialized_chunks);

    return (SerializedScene){combined_data, totalSize, gpu_chunks};
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
        .AddChunk = AddChunk,
        .Render = Render,
        .SerializeChunks = SerializeChunks,
        .WriteToFile = WriteToFile,
        .ReadFile = ReadFile,
};