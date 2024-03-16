/**
 * @file global.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-07-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include "../../input/input.h"
#include "../config/config.h"

typedef struct global
{
    Input_State input;
    Config_State config;
    // Time_State time;
} Global;

extern Global global;

#endif