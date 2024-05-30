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

#define GPUCHUNK_BUFFER_CACHE_SIZE 20
#define CHUNK_DATA_CACHE_SIZE 3000

static gpu_cache_manager_t *gpu_manager = NULL;

static gpu_cache_manager_t *Init()
{
    puts("[INFO] Initializing gpu cache");

    if (gpu_manager)
        ERROR_RETURN(NULL, "[WARNING] The gpu cache is already defined");

    gpu_manager = malloc(sizeof(gpu_cache_manager_t));
    if (!gpu_manager)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Cache Manager.");

    // allocate memory for the gpuchunks buffer on cpu
    gpu_manager->chunks_buffer = malloc(sizeof(gpu_cache_chunk_t *) * GPUCHUNK_BUFFER_CACHE_SIZE);
    if (!gpu_manager->chunks_buffer)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Chunks cache buffer")
    memset(gpu_manager->chunks_buffer, 0, sizeof(gpu_cache_chunk_t *) * GPUCHUNK_BUFFER_CACHE_SIZE);

    gpu_manager->chunks_buffer_size = GPUCHUNK_BUFFER_CACHE_SIZE;
    gpu_manager->chunks_buffer_index = 0;

    // allocate memory for gpu chunks buffer on the gpu
    glGenBuffers(1, &gpu_manager->gpu_chunks_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GPUCHUNK_BUFFER_CACHE_SIZE * sizeof(GPUChunk), 0, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gpu_manager->gpu_chunks_buffer);
    gpu_manager->gpu_chunks_buffer_size = GPUCHUNK_BUFFER_CACHE_SIZE;

    gpu_manager->chunks_valid = true;

    // allocate memory for the chunks data on cpu
    gpu_manager->chunks_data_buffer = malloc(sizeof(unsigned int) * CHUNK_DATA_CACHE_SIZE);
    if (!gpu_manager->chunks_data_buffer)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Chunks data buffer")

    gpu_manager->chunks_data_buffer_size = CHUNK_DATA_CACHE_SIZE;
    gpu_manager->chunks_data_buffer_index = 0;

    // allocate memory for the chunks data on gpu
    glGenBuffers(1, &gpu_manager->gpu_chunks_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_data_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, CHUNK_DATA_CACHE_SIZE * sizeof(unsigned int), 0, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, gpu_manager->gpu_chunks_data_buffer);
    gpu_manager->gpu_chunks_data_buffer_size = CHUNK_DATA_CACHE_SIZE;

    gpu_manager->chunks_data_valid = true;

    // allocate memory for the textures on gpu
    glGenTextures(1, &gpu_manager->gpu_textures_storage_buffer);
    glBindTexture(GL_TEXTURE_2D_ARRAY, gpu_manager->gpu_textures_storage_buffer);

    // allocate storage for the texture array
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 64, 64, 64 * GPUCHUNK_BUFFER_CACHE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindImageTexture(4, gpu_manager->gpu_textures_storage_buffer, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);

    gpu_manager->gpu_textures_storage_buffer_size = GPUCHUNK_BUFFER_CACHE_SIZE;

    gpu_manager->textures_valid = true;

    return gpu_manager;
}

// Add chunk to the cpu cache buffer
// and changes data in the data buffer
// and change the texture buffer and gpu buffer texture index
// the cpu buffers can be bigger in size but the gpu buffer need to adjust and swap chunks dynamically

// this function only adds the chunk to the cpu chunk buffer of all seen chunks
static void AddToCache(Chunk *chunk)
{
    // puts("[INFO] Checking if we can add chunk to gpu cache");

    if (!chunk)
    {
        // puts("[INFO] No chunk");
        return;
    }

    for (size_t chunk_index = 0; chunk_index < gpu_manager->chunks_buffer_index; chunk_index++)
    {
        gpu_cache_chunk_t *cache_chunk = gpu_manager->chunks_buffer[chunk_index];

        if (!cache_chunk)
            continue;

        if (cache_chunk->chunk->gpu_chunk->x == chunk->gpu_chunk->x && cache_chunk->chunk->gpu_chunk->y == chunk->gpu_chunk->y && cache_chunk->chunk->gpu_chunk->z == chunk->gpu_chunk->z)
        {
            // puts("[INFO] Chunk already in cache");
            return;
        }
    }

    // puts("[INFO] Adding chunk to gpu cache");

    // create gpu_cache_chunk from the chunk
    gpu_cache_chunk_t *gpu_chunk = malloc(sizeof(gpu_cache_chunk_t));
    memset(gpu_chunk, 0, sizeof(gpu_cache_chunk_t));

    if (gpu_manager->chunks_buffer_size <= gpu_manager->chunks_buffer_index)
    {
        // puts("[INFO] Realloc gpu cache chunk buffer");

        // resize the buffer to hold more chunks
        gpu_manager->chunks_buffer_size += GPUCHUNK_BUFFER_CACHE_SIZE;
        gpu_manager->chunks_buffer = realloc(gpu_manager->chunks_buffer, sizeof(gpu_cache_chunk_t *) * gpu_manager->chunks_buffer_size);

        if (!gpu_manager->chunks_buffer)
            ERROR_EXIT("[ERROR] Failed to allocate memory for gpu chunks buffer");
    }

    gpu_chunk->chunk = chunk;
    gpu_chunk->chunk_buffer_index = gpu_manager->chunks_buffer_index;

    // add the chunk to the cpu memory
    gpu_manager->chunks_buffer[gpu_manager->chunks_buffer_index] = gpu_chunk;
    gpu_manager->chunks_buffer_index++;
}

static gpu_cache_manager_t *Get()
{
    return gpu_manager;
}

// Prepare all gpu buffers
static void Prepare()
{
    // puts("[INFO] Preparing gpu cache...");
    // printf("%d\n", gpu_manager->chunks_buffer_index);

    size_t gpu_chunk_buffer_index = 0;

    for (size_t chunk_index = 0; chunk_index < gpu_manager->chunks_buffer_index; chunk_index++)
    {
        gpu_cache_chunk_t *cache_chunk = gpu_manager->chunks_buffer[chunk_index];
        if (!cache_chunk)
            continue;

        if (cache_chunk->on_gpu)
        {
            gpu_chunk_buffer_index++;
            continue;
        }

        if (gpu_chunk_buffer_index >= GPUCHUNK_BUFFER_CACHE_SIZE)
            ERROR_RETURN(NULL, "[WARNING] Gpu buffer smol");

        // somehow decide which chunk should be removed if the amount of chunks needed on the gpu is higher than we can store

        // prepare the GPUChunk
        cache_chunk->chunk->gpu_chunk->textureIndex = gpu_chunk_buffer_index;
        cache_chunk->chunk->gpu_chunk->offset = gpu_manager->gpu_chunks_data_buffer_index;

        if (!cache_chunk->serialized_chunk)
        {
            SerializedChunk serialized_chunk = AChunk.Serialize(cache_chunk->chunk);
            cache_chunk->serialized_chunk = malloc(sizeof(SerializedChunk));
            if (!cache_chunk->serialized_chunk)
                ERROR_EXIT("[ERROR] Failed to allocate memory for serialized chunk");

            memcpy(cache_chunk->serialized_chunk, &serialized_chunk, sizeof(SerializedChunk));
        }

        puts("[INFO] Adding chunk to gpu buffers");

        glBufferSubData(gpu_manager->gpu_chunks_buffer, sizeof(GPUChunk) * gpu_chunk_buffer_index, sizeof(GPUChunk), cache_chunk->chunk->gpu_chunk);
        glBufferSubData(gpu_manager->gpu_chunks_data_buffer, sizeof(unsigned int) * gpu_manager->gpu_chunks_data_buffer_index, sizeof(unsigned int) * cache_chunk->serialized_chunk->size, cache_chunk->serialized_chunk->data);
        gpu_manager->gpu_chunks_data_buffer_index += cache_chunk->serialized_chunk->size;

        glBindTexture(GL_TEXTURE_3D, cache_chunk->chunk->heightMap);
        uint8_t *data = (uint8_t *)malloc(64 * 64 * 64 * sizeof(uint8_t));
        glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_UNSIGNED_BYTE, data);

        glBindTexture(GL_TEXTURE_2D_ARRAY, gpu_manager->gpu_textures_storage_buffer);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, gpu_chunk_buffer_index * 64, 64, 64, 64, GL_RED, GL_UNSIGNED_BYTE, data);

        free(data);

        cache_chunk->on_gpu = true;
        puts("[INFO] edsadsa");
    }
}

// Clean up
static void Delete()
{
    puts("[INFO] Deleting gpu cache");

    for (size_t chunk_index = 0; chunk_index < gpu_manager->chunks_buffer_index; chunk_index++)
    {
        gpu_cache_chunk_t *cache_chunk = gpu_manager->chunks_buffer[chunk_index];

        if (!cache_chunk)
            continue;

        free(cache_chunk->gpu_chunk);
        free(cache_chunk->serialized_chunk->data);
        free(cache_chunk->serialized_chunk);

        free(cache_chunk);
    }

    free(gpu_manager->chunks_buffer);
    glDeleteBuffers(1, gpu_manager->gpu_chunks_buffer);

    free(gpu_manager->chunks_data_buffer);
    glDeleteBuffers(1, gpu_manager->gpu_chunks_data_buffer);

    glDeleteBuffers(1, gpu_manager->gpu_textures_storage_buffer);

    free(gpu_manager);

    return;
}

extern struct AGpuCache AGpuCache;
struct AGpuCache AGpuCache =
    {
        .Init = Init,
        .AddToCache = AddToCache,
        .Get = Get,
        .Prepare = Prepare,
        .Delete = Delete};