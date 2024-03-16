/**
 * @file util.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-06-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../common/types/types.h"

#define ERROR_EXIT(...)               \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    }
#define ERROR_RETURN(R, ...)          \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        return R;                     \
    }

ull generate_random_id();

#endif