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

#include <inttypes.h>
#include <stdbool.h>
#include <linmath.h>

#include "../serialized/serialized.h"
#include "../../util/util.h"

typedef struct Model Model;
struct Model
{
    ull id;

    bool is_valid;

    unsigned int verticies_count;
    float *verticies;

    unsigned int indicies_count;
    unsigned int *indicies;

    int uv_count;
    vec3 *uvs;

    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;

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