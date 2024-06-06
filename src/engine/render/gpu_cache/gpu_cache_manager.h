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

    unsigned int offset;

    // any additional data to determine if the chunk should be replaced

    bool on_gpu;
};

typedef struct gpu_cache_manager_t gpu_cache_manager_t;
struct gpu_cache_manager_t
{
    // Storage for where the chunks are positioned
    GLuint gpu_chunks_location_buffer;

    // Storage for all the chunks we have seen
    gpu_cache_chunk_t **chunks_buffer;
    size_t chunks_buffer_size;

    GLuint gpu_chunks_buffer;
    size_t gpu_chunks_buffer_size;

    bool chunks_valid;

    GLuint gpu_chunks_data_buffer;
    size_t gpu_chunks_data_buffer_size;

    bool chunks_data_valid;
};

struct AGpuCache
{
    gpu_cache_manager_t *(*Init)();

    void (*AddToCache)(Chunk *chunk);

    gpu_cache_manager_t *(*Get)();

    void (*Cull)();

    void (*Prepare)();

    void (*Delete)();
};

extern struct AGpuCache AGpuCache;

#endif