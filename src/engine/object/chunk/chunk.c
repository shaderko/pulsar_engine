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

    return chunk;
}

static void Add(Chunk *chunk, unsigned int x, unsigned int y, unsigned int z, unsigned char color, unsigned int uvs)
{
    // Add data to octree
    AOctree.Add(chunk->voxel_tree, x, y, z, color, uvs);
}

static void Serialize(void *data)
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