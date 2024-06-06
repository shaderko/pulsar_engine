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
#include <glad/glad.h>

typedef struct GPUChunk GPUChunk;
struct GPUChunk
{
    // position
    float x;
    float y;
    float z;

    // ofsset for the index of linearized octree buffer
    unsigned int offset;
};

// until we use an octree again we don't need to use serialization of the voxels because they are already stored in an accessbile way
// typedef struct
// {
//     unsigned int *data;
//     unsigned int size;
// } SerializedChunk;

typedef struct Chunk Chunk;
struct Chunk
{
    vec3 position;
    // for now we get rid of the octree and replace it by unsigned ints of where the voxels are 32x32x32
    // octree *voxel_tree;

    // array of unsigned ints 32x32 (32*32=1024) (1024*4=4096 bytes per chunk)
    unsigned int *voxels;

    // hold the serialized chunk data for fast access, release when either the chunk changes or we wont use this for a long time
    // SerializedChunk serialized_chunk;
};

/**
 * ******************************************
 * *        Voxel Representation # 1        *
 * ******************************************
 *
 * 00000000 10000000 00000000 00000000
 * ^        ^                        ^
 * end      voxel exists             start
 *
 *
 * 32 bits each bit represents if a voxel exists
 *
 *
 */

/**
 * ******************************************
 * *        Voxel Representation # 2        *
 * ******************************************
 *
 * 00000000 00000000 00000000 00000000
 *                            ^^^^^^^^
 *                            color
 *
 *
 */

struct AChunk
{
    Chunk *(*Init)(vec3 position);

    void (*Add)(Chunk *chunk, unsigned int x, unsigned int y, unsigned int z, unsigned char color);

    void (*Visualize)(Chunk *chunk);
};

extern struct AChunk AChunk;

#endif