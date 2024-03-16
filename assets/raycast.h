#pragma once

#include <linmath.h>

#include "wall.h"

typedef struct {
    vec2 source;
    vec2 destination;
    vec2 hit;
    double angle;
} Raycast;

typedef struct {
    Wall* walls;
    int walls_size;
    int src_pos_x;
    int src_pos_y;
    int index;
    vec2* segments;
    int size;
} CastRaysArguments;

Raycast raycast_init(vec2 src_pos, vec2 dst_pos, vec2 hit);
void sort_rays(vec2* rays, size_t rays_size, vec2 src_pos);
Raycast cast_ray(vec2 src_pos, vec2 dst_pos, vec2* segments, size_t segments_size, double angle_offset);
void cast_rays(void* args);
void draw_rays(Wall* walls, size_t size, float src_x, float src_y);
vec2* intersection(float r_px, float r_py, float r_dx, float r_dy, vec2* segments, size_t segment_size);
