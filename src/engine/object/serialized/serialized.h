/**
 * @file serialized.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-07-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SERIALIZED_H
#define SERIALIZED_H

typedef struct SerializedDerived SerializedDerived;
struct SerializedDerived
{
    int len;
    char *data;
};

#endif