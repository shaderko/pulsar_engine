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
#include "./voxel/voxel.h"

#define OBJECT_CAPACITY 1024

static Chunk *Init(unsigned int x, unsigned int y, unsigned int z)
{
    Chunk *chunk = malloc(sizeof(Chunk));
    if (!chunk)
        ERROR_EXIT("[Error] Failed to allocate chunk.\n");

    memset(chunk, 0, sizeof(Chunk));

    chunk->x = x;
    chunk->y = y;
    chunk->z = z;

    chunk->valid = true;

    return chunk;
}

static void Add(Chunk *chunk, Voxel *voxel)
{
    if (!chunk || !voxel)
        puts("[Error] Chunk couldn't be updated because either chunk or voxel is NULL.\n");

    if (chunk->size >= OBJECT_CAPACITY)
        puts("[Error] Chunk is full!\n");

    chunk->voxels = realloc(chunk->voxels, sizeof(Voxel) * (chunk->size + 1));
    if (!chunk->voxels)
        ERROR_EXIT("[Error] Failed to reallocate memory for chunk.\n");

    memcpy(chunk->objects[chunk->size], voxel, sizeof(Voxel));

    return;
}

static void Render(Chunk *chunk)
{
    if (!chunk)
        puts("[Error] Chunk couldn't be rendered because chunk is NULL.\n");

    if (!chunk->valid)
        puts("[Info] Chunk couldn't be rendered because chunk is invalid.\n");
}

extern struct AChunk AChunk;
struct AChunk AChunk =
    {
        .Init = Init,
        .Add = Add};