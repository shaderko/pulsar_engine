/**
 * @file octree.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdlib.h>
#include <stdbool.h>

#include "octree.h"
#include "../../../util/util.h"

#define OCTREE_SIZE 64
#define OCTREE_DEPTH 6

#define LEAF_BIT_MASK (1U << 31)
#define HAS_VERTEX_BIT_MASK (1U << 30)
#define COLOR_BIT_MASK (0xFF << 22) // Mask to isolate 8 bits for color
#define COLOR_BIT_SHIFT 22          // Shift needed to position color at bits 22 to 29
#define CHILDREN_SIZE_BIT_MASK (0xFFFF << 8)

static void visualize_octree(OctreeNode *node, int depth);
static int add_data(Octree *octree, OctreeNode *node, unsigned char current_depth, unsigned int x, unsigned int y, unsigned int z, unsigned char color);
static OctreeNode *create_node();

static Octree *Init()
{
    Octree *octree = malloc(sizeof(Octree));
    if (!octree)
        ERROR_EXIT("Failed to allocate memory for octree.\n");

    octree->depth = OCTREE_DEPTH;
    octree->root = create_node();

    // puts("[INFO] Octree initialized");

    return octree;
}

static void VisualizeOctree(Octree *octree)
{
    puts("[INFO] Visualizing octree");

    if (!octree)
        ERROR_EXIT("Octree is NULL.\n");

    // Visualize the octree
    visualize_octree(octree->root, 0);
}

// Helper function to print spaces for indentation
static void print_spaces(int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("  ");
    }
}

void print_binary(unsigned int num)
{
    for (int i = 31; i >= 0; i--)
    {
        printf("%u", (num >> i) & 1);
        if (i % 8 == 0 && i != 0)
        { // Add a space every 8 bits for readability
            printf(" ");
        }
    }
}

// Function to visualize the octree
static void visualize_octree(OctreeNode *node, int depth)
{
    if (!node)
        return;

    print_spaces(depth); // Indent based on depth
    printf("Node at depth %d: %s\n", depth, (node->data & (1 << 31)) ? "Leaf" : "Not Leaf");
    print_spaces(depth); // Indent based on depth
    print_binary(node->data);
    printf(")\n");

    if (!(node->data & (1 << 31)) && node->children)
    { // If not a leaf and children exist
        for (int i = 0; i < 8; i++)
        {
            if (node->children[i])
            { // Recursively visualize each child
                visualize_octree(node->children[i], depth + 1);
            }
        }
    }
}

static OctreeNode *create_node()
{
    OctreeNode *node = malloc(sizeof(OctreeNode));
    if (!node)
        ERROR_EXIT("Failed to allocate memory for octree node.\n");

    // Set the first bit to 0 because this is not a leaf node, and the second to 0 because this node doesn't have any vertexes
    node->data = 0;
    // unsigned int current_children_count = (node->data & CHILDREN_SIZE_BIT_MASK) >> 8;
    // current_children_count++;
    // node->data = (node->data & ~CHILDREN_SIZE_BIT_MASK) | ((current_children_count % 0x1FFFF) << 8);
    node->children = NULL;

    return node;
}

static int add_data(Octree *octree, OctreeNode *node, unsigned char current_depth, unsigned int x, unsigned int y, unsigned int z, unsigned char color)
{
    // puts("[INFO] Adding data to octree");

    if (octree->depth == current_depth)
    {
        // puts("[INFO] Max depth reached");

        // todo
        // if (node->data != 0)
        // {
        //     puts("[INFO] Leaf already exists");
        //     return NULL;
        // }

        // This is a leaf node, so we can add the data to the node
        // Set the first bit to 1 because this is a leaf node
        node->data = 0;
        node->data |= LEAF_BIT_MASK;

        // Store the color
        node->data |= (node->data & ~COLOR_BIT_MASK) | ((color << COLOR_BIT_SHIFT) & COLOR_BIT_MASK);

        // Store the uvs

        return 1;
    }

    // Translate the coordinates to select a child node to add
    unsigned int mid_point = OCTREE_SIZE >> (current_depth + 1); // Divide size by 2^(current_depth+1)
    unsigned int index = (x >= mid_point) + ((y >= mid_point) << 1) + ((z >= mid_point) << 2);

    if (node->children == NULL)
    {
        node->children = malloc(sizeof(OctreeNode *) * 8);
        if (!node->children)
            ERROR_EXIT("Failed to allocate memory for octree children.\n");

        memset(node->children, 0, sizeof(OctreeNode *) * 8);
    }

    if (node->children[index] == NULL)
    {
        // Set the corresponding bit to 1 to indicate that this child exists
        node->data |= (1 << index);

        node->children[index] = create_node(); // Lazy initialization of child nodes
    }

    add_data(octree, node->children[index], current_depth + 1, x % mid_point, y % mid_point, z % mid_point, color);

    int result = 1;
    for (int i = 0; i < 8; i++)
    {
        if (!node->children[i])
        {
            continue;
        }

        if (node->children[i]->data & LEAF_BIT_MASK)
        {
            result++;
        }

        result += (node->children[i]->data & CHILDREN_SIZE_BIT_MASK) >> 8;
    }

    unsigned int current_children_count = (node->data & CHILDREN_SIZE_BIT_MASK) >> 8;
    current_children_count = result;
    node->data = (node->data & ~CHILDREN_SIZE_BIT_MASK) | ((current_children_count % 0x1FFFF) << 8);

    return current_children_count;
}

static void Add(Octree *octree, unsigned int x, unsigned int y, unsigned int z, unsigned char color, unsigned int uvs)
{
    add_data(octree, octree->root, 0, x, y, z, color);
}

static void LinearizeOctree(OctreeNode *root, unsigned int **array, unsigned int *size)
{
    // puts("[INFO] Serializing octree.");

    if (root == NULL)
    {
        puts("[INFO] No root provided.");
        return;
    }

    unsigned int capacity = 300; // Initial capacity of the array
    *array = malloc(capacity * sizeof(unsigned int));
    *size = 0;

    // Queue for breadth-first traversal
    OctreeNode **stack = malloc(capacity * sizeof(OctreeNode *));
    int top = 0;

    // Enqueue root
    stack[top++] = root;

    while (top > 0)
    {
        OctreeNode *current = stack[--top];
        // puts("[INFO] Serializing...");

        // Resize queue and array if needed
        if (*size >= capacity - 1)
        {
            // puts("[INFO] Realloc");
            capacity *= 2;
            stack = realloc(stack, capacity * sizeof(OctreeNode *));
            *array = realloc(*array, capacity * sizeof(unsigned int));

            if (!(*array) || !stack)
            {
                puts("[ERROR] Memory reallocation failed.");
                return;
            }
        }

        // Process current node
        (*array)[(*size)++] = current->data;

        // Enqueue children this can be further enhanced because we know which children we have
        if (!(current->data & LEAF_BIT_MASK) && current->children)
        {
            for (int i = 7; i >= 0; i--)
            {
                if (current->data & (1 << i))
                {
                    // Check if the child exists
                    if (current->children[i] != NULL)
                    {
                        stack[top++] = current->children[i];
                    }
                    else
                    {
                        puts("[ERROR] Something unintended happened\n");
                    }
                }
            }
        }
    }

    free(stack);
}

extern struct AOctree AOctree;
struct AOctree AOctree =
    {
        .Init = Init,
        .print_binary = print_binary,
        .Add = Add,
        .VisualizeOctree = VisualizeOctree,
        .LinearizeOctree = LinearizeOctree};