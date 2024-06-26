/**
 * @file chunk.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef CHUNK_H
#define CHUNK_H

#include <linmath.h>
#include <stdbool.h>

#include "octree/octree.h"

typedef struct Chunk Chunk;
struct Chunk
{
    unsigned int position; // Max value of 1024, later can be upgraded to long unsigned int which is 8 bytes (2097151)
    Octree *voxel_tree;
};

typedef struct GPUChunk GPUChunk;
struct GPUChunk
{
    unsigned int position;
    unsigned int offset;
    unsigned int size;
    unsigned int valid;
};

typedef struct
{
    unsigned int *data;
    unsigned int size;
} SerializedChunk;

struct AChunk
{
    Chunk *(*Init)(vec3 position);

    void (*Add)(Chunk *chunk, unsigned int x, unsigned int y, unsigned int z, unsigned char color, unsigned int uvs);

    void (*Serialize)(void *data);
};

extern struct AChunk AChunk;

#endif