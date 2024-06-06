/**
 * @file gpu_cache_manager.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "gpu_cache_manager.h"
#include "../../util/util.h"

// Macro to get the number of bytes needed for given number of bits
#define NUM_BYTES(bits) (((bits) + 7) / 8)

#define MAX_CHUNKS 64                 // on one axis
#define GPUCHUNK_BUFFER_CACHE_SIZE 32 // this should be of dividable by 32
#define CHUNK_DATA_CACHE_SIZE 100000

static gpu_cache_manager_t *gpu_manager = NULL;

static gpu_cache_manager_t *Init()
{
    puts("[INFO] Initializing gpu cache");

    if (gpu_manager)
        ERROR_RETURN(NULL, "[WARNING] The gpu cache is already defined");

    gpu_manager = malloc(sizeof(gpu_cache_manager_t));
    if (!gpu_manager)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Cache Manager.");
    memset(gpu_manager, 0, sizeof(gpu_cache_manager_t));

    // allocate memory for the gpuchunks buffer on cpu
    gpu_manager->chunks_buffer = malloc(sizeof(gpu_cache_chunk_t *) * GPUCHUNK_BUFFER_CACHE_SIZE);
    if (!gpu_manager->chunks_buffer)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Chunks cache buffer")
    memset(gpu_manager->chunks_buffer, 0, sizeof(gpu_cache_chunk_t *) * GPUCHUNK_BUFFER_CACHE_SIZE);

    gpu_manager->chunks_buffer_size = GPUCHUNK_BUFFER_CACHE_SIZE;

    // allocate memory for where the chunks are on the gpu
    glGenBuffers(1, &gpu_manager->gpu_chunks_location_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_location_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned char) * MAX_CHUNKS * MAX_CHUNKS * MAX_CHUNKS, 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gpu_manager->gpu_chunks_location_buffer);

    // allocate memory for gpu chunks buffer on the gpu
    glGenBuffers(1, &gpu_manager->gpu_chunks_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GPUCHUNK_BUFFER_CACHE_SIZE * sizeof(unsigned int), 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gpu_manager->gpu_chunks_buffer);
    gpu_manager->gpu_chunks_buffer_size = GPUCHUNK_BUFFER_CACHE_SIZE;

    gpu_manager->chunks_valid = true;

    // allocate memory for the chunks data on gpu
    glGenBuffers(1, &gpu_manager->gpu_chunks_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_data_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, CHUNK_DATA_CACHE_SIZE * sizeof(unsigned int), 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gpu_manager->gpu_chunks_data_buffer);
    gpu_manager->gpu_chunks_data_buffer_size = CHUNK_DATA_CACHE_SIZE;

    gpu_manager->chunks_data_valid = true;

    return gpu_manager;
}

void gpu_set_byte(GLuint buffer, size_t index, unsigned char value)
{
    printf("[INFO] Setting byte %d to %d\n", index, value);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned char) * index, sizeof(unsigned char), &value);
}

static void AddToCache(Chunk *chunk)
{
    if (!chunk)
        return;

    int free_index = -1;

    // figure out if the chunk is already in the cpu cache
    for (size_t chunk_index_exists = 0; chunk_index_exists < gpu_manager->chunks_buffer_size; chunk_index_exists++)
    {
        gpu_cache_chunk_t *cache_chunk = gpu_manager->chunks_buffer[chunk_index_exists];

        if (!cache_chunk)
        {
            if (free_index < 0)
                free_index = chunk_index_exists;
            continue;
        }

        if (cache_chunk->chunk->position[0] == chunk->position[0] && cache_chunk->chunk->position[1] == chunk->position[1] && cache_chunk->chunk->position[2] == chunk->position[2])
            return;
    }

    // the chunk is not in the gpu cache figure out a place to add it to
    if (free_index >= 0)
    {
        printf("[INFO] Adding chunk to cache at position %d\n", free_index);

        gpu_cache_chunk_t *cache_chunk = gpu_manager->chunks_buffer[free_index];

        // create gpu_cache_chunk from the chunk
        gpu_cache_chunk_t *gpu_cache_chunk = malloc(sizeof(gpu_cache_chunk_t));
        memset(gpu_cache_chunk, 0, sizeof(gpu_cache_chunk_t));

        gpu_cache_chunk->chunk = chunk;
        gpu_cache_chunk->offset = free_index * 1024;

        // add the chunk to the cpu memory
        gpu_manager->chunks_buffer[free_index] = gpu_cache_chunk;

        printf("position: %i %i %i\n", (int)chunk->position[0], (int)chunk->position[1], (int)chunk->position[2]);
        printf("position index: %d\n", (int)chunk->position[0] + (int)chunk->position[1] * MAX_CHUNKS + (int)chunk->position[2] * MAX_CHUNKS * MAX_CHUNKS);

        // delete the existance bit in the gpu memory
        gpu_set_byte(gpu_manager->gpu_chunks_location_buffer, (int)chunk->position[0] + (int)chunk->position[1] * MAX_CHUNKS + (int)chunk->position[2] * MAX_CHUNKS * MAX_CHUNKS, free_index);

        return;
    }

    // there is no space left

    ERROR_EXIT("[ERROR] Failed to add chunk to gpu cache, out of memory");
}

static gpu_cache_manager_t *Get()
{
    return gpu_manager;
}

static void Cull()
{
    return;
}

// Prepare all gpu buffers
static void Prepare()
{
    puts("[INFO] Preparing");
    for (size_t chunk_index = 0; chunk_index < gpu_manager->chunks_buffer_size; chunk_index++)
    {
        gpu_cache_chunk_t *cache_chunk = gpu_manager->chunks_buffer[chunk_index];
        if (!cache_chunk || cache_chunk->on_gpu)
            continue;

        // somehow decide which chunk should be removed if the amount of chunks needed on the gpu is higher than we can store

        puts("[INFO] Adding chunk to gpu buffers");

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_buffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * chunk_index, sizeof(unsigned int), cache_chunk->offset);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_data_buffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * chunk_index * 1024, sizeof(unsigned int) * 1024, cache_chunk->chunk->voxels);

        cache_chunk->on_gpu = true;
    }
}

// Clean up
static void Delete()
{
    puts("[INFO] Deleting gpu cache");

    for (size_t chunk_index = 0; chunk_index < gpu_manager->chunks_buffer_size; chunk_index++)
    {
        gpu_cache_chunk_t *cache_chunk = gpu_manager->chunks_buffer[chunk_index];

        if (!cache_chunk)
            continue;

        free(cache_chunk);
    }

    free(gpu_manager->chunks_buffer);

    glDeleteBuffers(1, gpu_manager->gpu_chunks_location_buffer);

    glDeleteBuffers(1, gpu_manager->gpu_chunks_buffer);

    glDeleteBuffers(1, gpu_manager->gpu_chunks_data_buffer);

    free(gpu_manager);

    return;
}

extern struct AGpuCache AGpuCache;
struct AGpuCache AGpuCache =
    {
        .Init = Init,
        .AddToCache = AddToCache,
        .Get = Get,
        .Cull = Cull,
        .Prepare = Prepare,
        .Delete = Delete};