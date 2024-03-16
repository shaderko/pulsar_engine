#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "wall.h"
#include "../src/engine/common/global/global.h"

/*
 * dont use, this function doesn't initialize segments, or render
 */
Wall create_wall(float x, float y, float width, float height)
{
    vec2 *segments = malloc(sizeof(vec2) * 2 * 4);

    vec2 right_top = {x + width, y + height};
    vec2 right_bottom = {x + width, y};
    vec2 left_top = {x, y + height};
    vec2 left_bottom = {x, y};

    vec2 *points_array[] = {&left_top, &right_top, &right_bottom, &left_bottom};
    for (int x = 0; x < 4; x++)
    {
        vec2 *first_segment = points_array[x];
        vec2 *second_segment = points_array[(x + 1) % 4];

        memcpy(segments + (x * 2), first_segment, sizeof(vec2));
        memcpy(segments + ((x * 2) + 1), second_segment, sizeof(vec2));
    }

    Wall w = {{x + width / 2, y + height / 2}, {width, height}, segments, true, NULL, 0};
    return w;
}

/*
 * walls: takes all walls to make into segments
 *
 * size: is the length of walls in elements
 *
 * returns: pointer to array of arrays of vector 2 elements which specify segments
 */
vec2 *segments(Wall *walls, int size)
{
    // size + 1 is there because at the end, we add walls of window
    vec2 *wall_segments = malloc((size + 1) * sizeof(vec2) * 2 * 4);

    for (int i = 0; i < size; i++)
    {
        memcpy(wall_segments + (i * 2 * 4), walls[i].segments, sizeof(vec2) * 2 * 4);
    }

    // add segments of the end of screen
    vec2 left_top = {0, global.render.height};
    vec2 right_top = {global.render.width, global.render.height};
    vec2 right_bottom = {global.render.width, 0};
    vec2 left_bottom = {0, 0};

    vec2 *points_array[] = {&left_top, &right_top, &right_bottom, &left_bottom};

    for (int x = 0; x < 4; x++)
    {
        vec2 *first_segment = points_array[x];
        vec2 *second_segment = points_array[(x + 1) % 4];

        memcpy(wall_segments + ((size) * 4 * 2) + (x * 2), first_segment, sizeof(vec2));
        memcpy(wall_segments + ((size) * 4 * 2) + ((x * 2) + 1), second_segment, sizeof(vec2));
    }

    return wall_segments;
}

Wall random_wall(float width, float height)
{
    float x = ((float)rand() / (float)(RAND_MAX)) * global.render.width;
    float y = ((float)rand() / (float)(RAND_MAX)) * global.render.height;

    return create_wall(x, y, width, height);
}

Wall *create_random_walls(int size, float width, float height)
{
    Wall *walls = malloc(sizeof(Wall) * size);

    for (int i = 0; i < size; i++)
    {
        walls[i] = random_wall(width, height);
    }

    return walls;
}

Wall *update_walls(Wall *walls, int *size, Wall *new_walls, int new_walls_size)
{
    walls = realloc(walls, sizeof(Wall) * ((*size) + new_walls_size));

    for (int i = 0; i < new_walls_size; i++)
    {
        walls[i + (*size)] = new_walls[i];
    }

    *size += new_walls_size;

    return walls;
}

void draw_walls(Wall *walls, size_t walls_size)
{
    for (int i = 0; i < walls_size; i++)
    {
        if (walls[i].render)
        {
            render_quad(walls[i].position, walls[i].size, (vec4){1, 1, 1, 1}, true);
        }
    }
}
