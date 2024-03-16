#pragma once

#include <linmath.h>
#include <stdbool.h>

typedef struct wall {
    vec2 position;
    vec2 size;
    vec2* segments;
    bool render;
    vec2* rays;
    size_t rays_size;
} Wall;

Wall* create_random_walls(int size, float width, float height);
Wall random_wall(float width, float height);
Wall* update_walls(Wall* walls, int* size, Wall* new_walls, int new_walls_size);
Wall create_wall(float x, float y, float width, float height);
vec2* segments(Wall* walls, int size);
Wall random_wall(float width, float height);
void draw_walls(Wall* walls, size_t walls_size);
