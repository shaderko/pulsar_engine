/**
 * @file render_init.h
 * @author https://github.com/shaderko
 * @brief Initialization functions for render.
 * @version 0.1
 * @date 2024-02-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef RENDER_INIT_H
#define RENDER_INIT_H

#include "../common/types/types.h"
#include "render.h"

void render_init_shaders(WindowRender *render);
u32 render_shader_create(const char *path_vert, const char *path_frag);
u32 render_shader_create_name(char *path);

#endif