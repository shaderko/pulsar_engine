/**
 * @file octree.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef OCTREE_H
#define OCTREE_H

typedef struct OctreeNode OctreeNode;
struct OctreeNode
{
    unsigned int data; // Holds in the first bit if its leaf or not, then the rest is for voxel data
    struct OctreeNode **children;
};

typedef struct Octree Octree;
struct Octree
{
    unsigned char depth; // Max value of 255
    OctreeNode *root;
};

struct AOctree
{
    Octree *(*Init)();
    void (*print_binary)(unsigned int num);
    void (*Add)(Octree *octree, unsigned int x, unsigned int y, unsigned int z, unsigned char color, unsigned int uvs);
    void (*VisualizeOctree)(Octree *octree);
    void (*LinearizeOctree)(OctreeNode *root, unsigned int **array, unsigned int *size);
};

extern struct AOctree AOctree;

#endif