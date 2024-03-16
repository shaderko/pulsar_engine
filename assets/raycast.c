#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <stdio.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#include "raycast.h"
#include "../src/engine/render/render.h"

#define radians(a) (a * 180.0 / M_PI)


Raycast raycast_init(vec2 src_pos, vec2 dst_pos, vec2 hit) {
    double angle = atan2(hit[1] - src_pos[1], hit[0] - src_pos[0]);
    Raycast ray = {{*src_pos, *dst_pos}, {*hit, angle}};
    return ray;
}


void sort_rays(vec2* rays, size_t rays_size, vec2 src_pos) {
    vec2* temp = malloc(sizeof(vec2));
    for (int i = 0; i < rays_size; i++) {
        double angle = atan2(rays[i][1] - src_pos[1], rays[i][0] - src_pos[0]);

        for (int x = 0; x < i; x++) {
            double angle_prev = atan2(rays[i - x - 1][1] - src_pos[1], rays[i - x - 1][0] - src_pos[0]);

            if (angle_prev > angle) {
                // swap
                memcpy(temp, &rays[i - x - 1], sizeof(vec2));
                memcpy(&rays[i - x - 1], &rays[i - x], sizeof(vec2));
                memcpy(&rays[i - x], temp, sizeof(vec2));
            } else {
                break;
            }
        }
    }
}

bool is_angle_within_range(double higher_angle, double lower_angle, double angle_to_check) {
    if (higher_angle > M_PI * .5 && lower_angle < -(M_PI * .5)) {
        return (angle_to_check <= M_PI && angle_to_check >= higher_angle) || (angle_to_check <= lower_angle && angle_to_check >= -M_PI);
    }
    return angle_to_check >= lower_angle && angle_to_check <= higher_angle;
}


Raycast cast_ray(vec2 src_pos, vec2 dst_pos, vec2* segments, size_t segments_size, double angle_offset) {
    float src_pos_x = src_pos[0];
    float src_pos_y = src_pos[1];

    float dst_pos_x = dst_pos[0];
    float dst_pos_y = dst_pos[1];

    vec2* hit = intersection(src_pos_x, src_pos_y, dst_pos_x - src_pos_x + angle_offset, dst_pos_y - src_pos_y + angle_offset, segments, segments_size);
    Raycast ray = raycast_init(src_pos, dst_pos, *hit);
    return ray;
}


void cast_rays(void* args) {
    CastRaysArguments* cast_rays_args = (CastRaysArguments*)args;
    Wall* walls = cast_rays_args->walls;
    int walls_size = cast_rays_args->walls_size;
    int src_pos_x = cast_rays_args->src_pos_x;
    int src_pos_y = cast_rays_args->src_pos_y;
    int index = cast_rays_args->index;
    vec2* segments = cast_rays_args->segments;
    int size = cast_rays_args->size;

    if (!walls[index].render) {
        return;
    }
    size_t segments_size = size * 4;

    size_t rays_size = 0;
    vec2* rays = malloc(0);

    double higher_hit;
    double lower_hit;
    bool lower_hit_check = false;
    bool higher_hit_check = false;

    for (int i = 0; i < 4; i++) {
        float dst_pos_x = walls[index].segments[i * 2][0];
        float dst_pos_y = walls[index].segments[i * 2][1];

        vec2* lower = intersection(src_pos_x, src_pos_y, dst_pos_x - src_pos_x - radians(0.001), dst_pos_y - src_pos_y - radians(0.001), segments, segments_size);
        vec2* straight = intersection(src_pos_x, src_pos_y, dst_pos_x - src_pos_x, dst_pos_y - src_pos_y, segments, segments_size);
        vec2* higher = intersection(src_pos_x, src_pos_y, dst_pos_x - src_pos_x + radians(0.001), dst_pos_y - src_pos_y + radians(0.001), segments, segments_size);

        if (lower != NULL) {
            double lower_angle = atan2(lower[0][1] - src_pos_y, lower[0][0] - src_pos_x);
            if (!lower_hit_check || lower_hit > lower_angle) {
                lower_hit_check = true;
                lower_hit = lower_angle;
            } else if (!higher_hit_check || higher_hit < lower_angle) {
                higher_hit_check = true;
                higher_hit = lower_angle;
            }

            if (lower[0][0] != dst_pos_x && lower[0][1] != dst_pos_y) {
                rays = realloc(rays, sizeof(vec2) * (rays_size + 1));
                memcpy(rays + rays_size, lower, sizeof(vec2));
                rays_size++;
            }
            free(lower);
        }
        if (straight != NULL) {
            rays = realloc(rays, sizeof(vec2) * (rays_size + 1));
            memcpy(rays + rays_size, straight, sizeof(vec2));
            free(straight);
            rays_size++;
        }
        if (higher != NULL) {
            double higher_angle = atan2(higher[0][1] - src_pos_y, higher[0][0] - src_pos_x);
            if (!higher_hit_check || higher_hit < higher_angle) {
                higher_hit_check = true;
                higher_hit = higher_angle;
            } else if (!lower_hit_check || lower_hit > higher_angle) {
                lower_hit_check = true;
                lower_hit = higher_angle;
            }

            if (higher[0][0] != dst_pos_x && higher[0][1] != dst_pos_y) {
                rays = realloc(rays, sizeof(vec2) * (rays_size + 1));
                memcpy(rays + rays_size, higher, sizeof(vec2));
                rays_size++;
            }
            free(higher);
        }
    }

    if (lower_hit_check && higher_hit_check) {
        for (int i = 0; i < walls_size; i++) {
            if (i == index) {
                continue;
            }
            vec2* wall_segment = walls[i].segments;
            bool is_outside = false;
            for (int x = 0; x < 4; x++) {
                double angle = atan2(wall_segment[x * 2][1] - src_pos_y, wall_segment[x * 2][0] - src_pos_x);

                if (is_angle_within_range(higher_hit, lower_hit, angle)) {
                    is_outside = true;
                } else {
                    is_outside = false;
                    break;
                }
            }
            if (is_outside) {
                walls[i].render = false;
            }
        }
    }

    walls[index].rays = rays;
    walls[index].rays_size = rays_size;

    return;
}

void draw_rays(Wall* walls, size_t size, float src_x, float src_y) {
    size_t overall_size = 0;
    vec2* rays = malloc(0);

    for (int i = 0; i < size; i++) {
        if (!walls[i].render) {
            if (walls[i].rays_size > 0) {
                free(walls[i].rays);
            }
            walls[i].rays_size = 0;
        }
        for (int x = 0; x < walls[i].rays_size; x++) {
            if (!walls[i].render) {
                walls[i].rays_size = 0;
                free(walls[i].rays);
                break;
            }
            render_line((vec2){src_x, src_y}, walls[i].rays[x], (vec3){1, 1, 1});
            render_quad(walls[i].rays[x], (vec2){5, 5}, (vec4){1, 0, 0, 1}, true);
        }
        if (walls[i].rays_size > 0) {
            rays = realloc(rays, sizeof(vec2) * (overall_size + walls[i].rays_size));
            memcpy(rays + overall_size, walls[i].rays, walls[i].rays_size * sizeof(vec2));
            free(walls[i].rays);
            overall_size += walls[i].rays_size;
        }
        walls[i].render = true;
        walls[i].rays_size = 0;
    }

    sort_rays(rays, overall_size, (vec2){src_x, src_y});
    vec2* start_mouse = malloc(sizeof(vec2) * (overall_size + 1));
    memcpy(start_mouse + 1, rays, sizeof(vec2) * overall_size);
    memcpy(start_mouse, &(vec2){src_x, src_y}, sizeof(vec2));
    render_poly(start_mouse, overall_size + 1, (vec3){0.215, 0.215, 0.215});
    free(start_mouse);
    free(rays);
}

vec2* intersection(float r_px, float r_py, float r_dx, float r_dy, vec2* segments, size_t segment_size) {
    bool is_null = true;
    vec2* closest_intersect = NULL;
    float closest_distance = FLT_MAX;

    for (vec2* segment = segments; segment < segments + segment_size*2; segment += 2) {
        float segment_src_x = segment[0][0];
        float segment_src_y = segment[0][1];
        float segment_dst_x = segment[1][0];
        float segment_dst_y = segment[1][1];

        float s_dx = segment_dst_x - segment_src_x;
        float s_dy = segment_dst_y - segment_src_y;

        if (r_dx * s_dy == r_dy * s_dx) {
            // lines are parallel
            continue;
        }

        float T2 = (r_dx * (segment_src_y - r_py) + r_dy * (r_px - segment_src_x)) / (s_dx * r_dy - s_dy * r_dx);
        if (T2 < 0 || T2 > 1) {
            // intersection point is outside of segment
            continue;
        }

        float T1 = (segment_src_x + s_dx * T2 - r_px) / r_dx;
        if (T1 < 0) {
            // intersection point is behind ray origin
            continue;
        }

        if (T1 < closest_distance) {
            closest_distance = T1;
            if (is_null) {
                closest_intersect = malloc(sizeof(vec2));
            }
            memcpy(closest_intersect, &(vec2){r_px + r_dx * T1, r_py + r_dy * T1}, sizeof(vec2));
            is_null = false;
        }
    }

    return closest_intersect;
}
