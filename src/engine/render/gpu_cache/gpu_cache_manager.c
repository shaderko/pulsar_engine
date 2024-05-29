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
#define GPU_TEXTURE_CACHE_SIZE 20

static gpu_cache_manager_t *Init()
{
    gpu_cache_manager_t *gpu_manager = malloc(sizeof(gpu_cache_manager_t));
    if (!gpu_manager)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Cache Manager.");

    // allocate memory for the gpuchunks buffer on cpu
    gpu_manager->chunks_buffer = malloc(sizeof(gpu_cache_chunk_t) * GPUCHUNK_BUFFER_CACHE_SIZE);
    if (!gpu_manager->chunks_buffer)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Chunks cache buffer")

    gpu_manager->chunks_buffer_size = GPUCHUNK_BUFFER_CACHE_SIZE;

    // allocate memory for gpu chunks buffer on the gpu
    glGenBuffers(1, &gpu_manager->gpu_chunks_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, GPUCHUNK_BUFFER_CACHE_SIZE * sizeof(GPUChunk), NULL, GL_STATIC_DRAW);
    gpu_manager->gpu_chunks_buffer_size = GPUCHUNK_BUFFER_CACHE_SIZE;

    gpu_manager->chunks_valid = true;

    // allocate memory for the chunks data on cpu
    gpu_manager->chunks_data_buffer = malloc(sizeof(unsigned int) * CHUNK_DATA_CACHE_SIZE);
    if (!gpu_manager->chunks_data_buffer)
        ERROR_EXIT("[ERROR] Failed to allocate memory for GPU Chunks data buffer")

    gpu_manager->chunks_data_buffer_size = CHUNK_DATA_CACHE_SIZE;

    // allocate memory for the chunks data on gpu
    glGenBuffers(1, &gpu_manager->gpu_chunks_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_manager->gpu_chunks_data_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, CHUNK_DATA_CACHE_SIZE * sizeof(unsigned int), NULL, GL_STATIC_DRAW);
    gpu_manager->gpu_chunks_data_buffer_size = CHUNK_DATA_CACHE_SIZE;

    gpu_manager->chunks_data_valid = true;

    // allocate memory for the textures on gpu
    glGenTextures(1, &gpu_manager->gpu_textures_storage_buffer);
    glBindTexture(GL_TEXTURE_2D_ARRAY, gpu_manager->gpu_textures_storage_buffer);

    // allocate storage for the texture array
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 64, 64, 64 * GPU_TEXTURE_CACHE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    gpu_manager->gpu_textures_storage_buffer_size = GPU_TEXTURE_CACHE_SIZE;

    gpu_manager->textures_valid = true;

    return gpu_manager;
}

// TODO: first change the shader to work with vec3 for position

// Add chunk to the cpu cache buffer
// and changes data in the data buffer
// and change the texture buffer and gpu buffer texture index

// the cpu buffers can be bigger in size but the gpu buffer need to adjust and swap chunks dynamically

// Prepare all gpu buffers

// Clean up