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

#define MAX_CHUNKS 1024

static Chunk *Init(vec3 position)
{
    Chunk *chunk = malloc(sizeof(Chunk));
    if (!chunk)
        ERROR_EXIT("[Error] Failed to allocate chunk.\n");
    memset(chunk, 0, sizeof(Chunk));

    // copy the position
    memcpy(chunk->position, position, sizeof(vec3));

    // create the voxel list and make it empty
    chunk->voxels = malloc(sizeof(unsigned int) * 1024);
    memset(chunk->voxels, 0, sizeof(unsigned int) * 1024);

    return chunk;
}

static void Add(Chunk *chunk, unsigned int x, unsigned int y, unsigned int z, unsigned char color)
{
    // check if the position is within the chunk
    if (x > 31 || y > 31 || z > 31)
        ERROR_RETURN(NULL, "[WARNING] Position is out of bounds.\n");

    // get the index of the voxel by multiplying the y position with the size of the chunk
    unsigned int index = x + z * 32;

    chunk->voxels[index] |= 1 << y;
}

static void Visualize(Chunk *chunk)
{
    puts("[INFO] Visualizing chunk");

    if (!chunk)
        ERROR_RETURN(NULL, "[WARNING] Chunk is NULL.\n");

    // Visualize the chunk
    for (int z_ = 0; z_ < 32; z_++)
    {
        printf("Layer %i:\n", z_);
        for (int x_ = 0; x_ < 32; x_++)
        {
            for (int y_ = 0; y_ < 32; y_++)
            {
                if (chunk->voxels[x_ + z_ * 32] & (1 << y_))
                    printf("1");
                else
                    printf("0");
            }
            printf("\n");
        }
    }
}

extern struct AChunk AChunk;
struct AChunk AChunk =
    {
        .Init = Init,
        .Add = Add,
        .Visualize = Visualize};