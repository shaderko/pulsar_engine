/**
 * @file model.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-07-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef MODEL_H
#define MODEL_H

#include <stdbool.h>
#include <linmath.h>

#include "../serialized/serialized.h"

typedef struct Model Model;
struct Model
{
    bool is_valid;

    unsigned int verticies_count;
    float *verticies;

    unsigned int indicies_count;
    unsigned int *indicies;

    int uv_count;
    vec3 *uvs;

    mat4x4 transform;

    vec4 color;
};

struct AModel
{
    Model *(*Init)();
    void (*Delete)(Model *model);
    Model *(*InitBox)();
    Model *(*InitMesh)(int verticies_count, vec3 *verticies, int indicies_count, unsigned int *indicies, int uv_count, vec3 *uvs, vec4 color);
    Model *(*Load)(const char *path);
    SerializedDerived (*Serialize)(Model *model);
    Model *(*Deserialize)(SerializedDerived serialized);
};

extern struct AModel AModel[1];

#endif