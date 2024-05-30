/**
 * @file gpu_cache_manager.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef GPU_CACHE_MANAGER_H
#define GPU_CACHE_MANAGER_H

#include <time.h>
#include <stdbool.h>

#include "../../object/chunk/chunk.h"

typedef struct gpu_cache_chunk_t gpu_cache_chunk_t;
struct gpu_cache_chunk_t
{
    Chunk *chunk;
    GPUChunk *gpu_chunk;
    SerializedChunk *serialized_chunk;

    // any additional data to determine if the chunk should be replaced
    size_t chunk_buffer_index;
    size_t chunk_data_buffer_index;

    bool on_gpu;
};

typedef struct gpu_cache_manager_t gpu_cache_manager_t;
struct gpu_cache_manager_t
{
    // Storage for the chunks and chunk data
    gpu_cache_chunk_t **chunks_buffer;
    size_t chunks_buffer_size;
    size_t chunks_buffer_index;

    GLuint gpu_chunks_buffer;
    size_t gpu_chunks_buffer_size;
    size_t gpu_chunks_buffer_index;

    bool chunks_valid;

    unsigned int *chunks_data_buffer;
    size_t chunks_data_buffer_size;
    size_t chunks_data_buffer_index;

    GLuint gpu_chunks_data_buffer;
    size_t gpu_chunks_data_buffer_size;
    size_t gpu_chunks_data_buffer_index;

    bool chunks_data_valid;

    // Storage for textures
    GLuint gpu_textures_storage_buffer;
    size_t gpu_textures_storage_buffer_size;

    bool textures_valid;
};

struct AGpuCache
{
    gpu_cache_manager_t *(*Init)();

    void (*AddToCache)(Chunk *chunk);

    gpu_cache_manager_t *(*Get)();

    void (*Prepare)();

    void (*Delete)();
};

extern struct AGpuCache AGpuCache;

#endif