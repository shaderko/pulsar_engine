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

#include "../object.h"

typedef struct Chunk Chunk;
struct Chunk
{
    Object **objects;
    size_t size;
    size_t size_capacity;

    unsigned int x;
    unsigned int y;
    unsigned int z;

    bool valid;
};

struct AChunk
{
    Chunk *(*Init)(unsigned int x, unsigned int y, unsigned int z);

    void (*Add)(Chunk *chunk, Object *object);

    void (*Render)(Chunk *chunk);
};

extern struct AChunk AChunk;

#endif