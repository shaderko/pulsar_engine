/**
 * @file chunk.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "chunk.h"
#include "../../util/util.h"
#include "../model/model.h"
#include "../../render/render.h"
#include "octree/octree.h"

#define MAX_CHUNKS 1024

#define X_MASK (0xA << 20)
#define Y_MASK (0xA << 10)
#define Z_MASK (0xA)

static Chunk *Init(vec3 position)
{
    Chunk *chunk = malloc(sizeof(Chunk));
    if (!chunk)
        ERROR_EXIT("[Error] Failed to allocate chunk.\n");

    // Transfer vec3 position to single unsigned int with bit manipulation
    chunk->position = (unsigned int)position[0] << 20 | (unsigned int)position[1] << 10 | (unsigned int)position[2];

    // Initialize an octree for this chunk
    chunk->voxel_tree = AOctree.Init();

    // Generate a height map for this chunk
    glGenTextures(1, &chunk->heightMap);
    glBindTexture(GL_TEXTURE_3D, chunk->heightMap);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, 64, 64, 64, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create dummy data for initial texture
    uint8_t *data = (uint8_t *)malloc(64 * 64 * 64 * sizeof(uint8_t));
    memset(data, 0, 64 * 64 * 64 * sizeof(uint8_t));
    for (int i = 0; i < 64 * 64 * 64; i++)
    {
        data[i] = 0;
    }
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 64, 64, 64, GL_RED, GL_UNSIGNED_BYTE, data);

    // Create the gpu chunk
    chunk->gpu_chunk = malloc(sizeof(GPUChunk));
    if (!chunk->gpu_chunk)
        ERROR_EXIT("[Error] Failed to allocate gpu chunk.\n");

    // Set the gpu chunk position
    chunk->gpu_chunk->position = chunk->position;
    // Set the gpu chunk texture index
    chunk->gpu_chunk->textureIndex = 0;

    return chunk;
}

static void Add(Chunk *chunk, unsigned int x, unsigned int y, unsigned int z, unsigned char color, unsigned int uvs)
{
    // Add data to octree
    AOctree.Add(chunk->voxel_tree, x, y, z, color, uvs);
}

static SerializedChunk Serialize(Chunk *chunk)
{
    SerializedChunk serialized_chunk;

    serialized_chunk.data = NULL;
    serialized_chunk.size = 0;

    // Linearize octree of the chunk
    AOctree.LinearizeOctree(chunk->voxel_tree->root, &serialized_chunk.data, &serialized_chunk.size);

    return serialized_chunk;
}

static void SerializeAsync(void *data)
{
    struct ThreadData
    {
        Chunk *chunk;
        SerializedChunk *result;
    } *thread_data = data;

    thread_data->result->data = NULL;
    thread_data->result->size = 0;

    if (!thread_data->chunk || !thread_data->chunk->voxel_tree)
        ERROR_RETURN(NULL, "[INFO] Chunk has no data to serialize.");

    // Linearize octree of the chunk
    AOctree.LinearizeOctree(thread_data->chunk->voxel_tree->root, &thread_data->result->data, &thread_data->result->size);
}

extern struct AChunk AChunk;
struct AChunk AChunk =
    {
        .Init = Init,
        .Add = Add,
        .Serialize = Serialize};