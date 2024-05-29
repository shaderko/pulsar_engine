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

#include "../../object/chunk/chunk.h"

typedef struct gpu_cache_chunk_t gpu_cache_chunk_t;
struct gpu_cache_chunk_t
{
    Chunk *chunk;

    // any additional data to determine if the chunk should be replaced
};

typedef struct gpu_cache_manager_t gpu_cache_manager_t;
struct gpu_cache_manager_t
{
    // Storage for the chunks and chunk data
    GPUChunk *chunks_buffer;
    size_t chunks_buffer_size;

    GLuint gpu_chunks_buffer;
    size_t gpu_chunks_buffer_size;

    bool chunks_valid;

    unsigned int *chunks_data_buffer;
    size_t chunks_data_buffer_size;

    GLuint gpu_chunks_data_buffer;
    size_t gpu_chunks_data_buffer_size;

    bool chunks_data_valid;

    // Storage for textures
    GLuint gpu_textures_storage_buffer;
    size_t gpu_textures_storage_buffer_size;

    bool textures_valid;
};

struct AGpuCache
{
    gpu_cache_manager_t *(*Init)();
};

#endif