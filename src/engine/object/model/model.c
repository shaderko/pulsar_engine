/**
 * @file model.c
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-07-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "model.h"

#include <stdio.h>
#include <stdlib.h>

#include "../../util/util.h"

// Init function should be called to load from cache
static Model *Init(vec4 color)
{
    Model *model = malloc(sizeof(Model));

    memcpy(model->color, color, sizeof(vec4));

    return model;
}

static void Delete(Model *model)
{
    free(model->verticies);
    free(model->indicies);
    free(model->uvs);
    free(model);
}

static Model *InitBox()
{
    Model *model = AModel->Init((vec4){1, 1, 1, 1});

    model->verticies_count = 8;
    model->verticies = malloc(sizeof(vec3) * model->verticies_count);
    memcpy(model->verticies[0], (vec3){-1, -1, -1}, sizeof(vec3));
    memcpy(model->verticies[1], (vec3){1, -1, -1}, sizeof(vec3));
    memcpy(model->verticies[2], (vec3){1, 1, -1}, sizeof(vec3));
    memcpy(model->verticies[3], (vec3){-1, 1, -1}, sizeof(vec3));
    memcpy(model->verticies[4], (vec3){-1, -1, 1}, sizeof(vec3));
    memcpy(model->verticies[5], (vec3){1, -1, 1}, sizeof(vec3));
    memcpy(model->verticies[6], (vec3){1, 1, 1}, sizeof(vec3));
    memcpy(model->verticies[7], (vec3){-1, 1, 1}, sizeof(vec3));

    model->indicies_count = 24;
    model->indicies = malloc(sizeof(unsigned int) * model->indicies_count);
    if (!model->indicies)
    {
        ERROR_EXIT("Couldn't allocate memory for indicies!\n");
    }

    model->uv_count = 0;
    model->uvs = NULL;

    unsigned int indicesArray[] = {0, 1, 2, 0, 2, 3, 1, 5, 6, 1, 6, 2, 5, 4, 7, 5, 7, 6, 4, 0, 3, 4, 3, 7};
    memcpy(model->indicies, indicesArray, sizeof(unsigned int) * model->indicies_count);

    model->is_valid = true;

    return model;
}

static Model *InitMesh(int verticies_count, vec3 *verticies, int indicies_count, unsigned int *indicies, int uv_count, vec3 *uvs, vec4 color)
{
    Model *model = AModel->Init(color);

    model->verticies_count = verticies_count;
    model->verticies = malloc(sizeof(vec3) * model->verticies_count);
    memcpy(model->verticies, verticies, sizeof(vec3) * model->verticies_count);

    model->indicies_count = indicies_count;
    model->indicies = malloc(sizeof(unsigned int) * model->indicies_count);
    memcpy(model->indicies, indicies, sizeof(unsigned int) * model->indicies_count);

    model->uv_count = uv_count;
    model->uvs = malloc(sizeof(vec3) * model->uv_count);
    memcpy(model->uvs, uvs, sizeof(vec3) * model->uv_count);

    model->is_valid = true;

    return model;
}

static Model *Load(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        printf("Failed to open file: %s\n", path);
        return NULL;
    }

    Model *model = malloc(sizeof(Model));
    if (!model)
    {
        printf("Failed to allocate memory for model\n");
        fclose(file);
        return NULL;
    }

    model->verticies = NULL;
    model->verticies_count = 0;
    model->indicies = NULL;
    model->indicies_count = 0;
    model->uvs = NULL;
    model->uv_count = 0;
    model->is_valid = false;

    char lineHeader[128];
    while (fscanf(file, "%s", lineHeader) != EOF)
    {
        if (strcmp(lineHeader, "v") == 0)
        {
            // Vertices
            model->verticies = realloc(model->verticies, sizeof(vec3) * (model->verticies_count + 1));
            if (!model->verticies)
            {
                printf("Failed to allocate memory for verticies\n");
                fclose(file);
                return NULL;
            }

            fscanf(file, "%f %f %f", &model->verticies[model->verticies_count][0], &model->verticies[model->verticies_count][1], &model->verticies[model->verticies_count][2]);
            model->verticies_count++;
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            // UVs
            // temp_uvs = realloc(temp_uvs, sizeof(vec3) * (uvIndex + 1));
            // fscanf(file, "%f %f %f", &temp_uvs[uvIndex][0], &temp_uvs[uvIndex][1], &temp_uvs[uvIndex][2]);
            // printf("UV: %f %f %f\n", temp_uvs[uvIndex][0], temp_uvs[uvIndex][1], temp_uvs[uvIndex][2]);
            // uvIndex++;
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            // Faces
            unsigned int vertexIndex[3], uvIndex[3];
            // int matches = fscanf(file, "%d/%d/%*d %d/%d/%*d %d/%d/%*d\n",
            //                      &vertexIndex[0], &uvIndex[0],
            //                      &vertexIndex[1], &uvIndex[1],
            //                      &vertexIndex[2], &uvIndex[2]);

            int matches = fscanf(file, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*dn",
                                 &vertexIndex[0],
                                 &vertexIndex[1],
                                 &vertexIndex[2]);

            model->indicies = realloc(model->indicies, sizeof(unsigned int) * (model->indicies_count + 3));
            if (!model->indicies)
            {
                printf("Failed to allocate memory for indicies\n");
                fclose(file);
                return NULL;
            }
            for (int i = 0; i < 3; i++)
            {
                model->indicies[model->indicies_count + i] = vertexIndex[i] - 1; // Indices are 1-based in .obj
            }
            model->indicies_count += 3;

            if (matches != 3)
            {
                printf("File can't be read by our simple parser\n");
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);

    model->is_valid = true;

    // You may initialize the position, size, rotation, and color in here, or outside this function.
    memcpy(model->color, (vec4){1, 1, 1, 1}, sizeof(vec4)); // TODO:

    puts("Model loaded successfully");
    printf("Vert count: %i, indicies count: %i, uv count: %i\n", model->verticies_count, model->indicies_count, model->uv_count);

    return model;
}

static SerializedDerived Serialize(Model *model)
{
    SerializedDerived serialized = {0};
    serialized.len = sizeof(Model) + (sizeof(vec3) * model->verticies_count) + (sizeof(unsigned int) * model->indicies_count) + (sizeof(vec3) * model->uv_count);

    printf("Serialized model size: %i bytes\n", serialized.len);

    serialized.data = malloc(serialized.len);
    if (!serialized.data)
    {
        ERROR_EXIT("Couldn't allocate memory for serialized model!\n");
    }

    // memcpy((char *)serialized.data, (char *)model, sizeof(Model)); // TODO:

    memcpy((char *)serialized.data, (char *)&model->is_valid, sizeof(bool));
    memcpy((char *)serialized.data + sizeof(bool), (char *)&model->verticies_count, sizeof(int));
    memcpy((char *)serialized.data + sizeof(bool) + sizeof(int), (char *)model->verticies, sizeof(vec3) * model->verticies_count);
    memcpy((char *)serialized.data + sizeof(bool) + sizeof(int) + sizeof(vec3) * model->verticies_count, (char *)&model->indicies_count, sizeof(int));
    memcpy((char *)serialized.data + sizeof(bool) + sizeof(int) + sizeof(vec3) * model->verticies_count + sizeof(int), (char *)model->indicies, sizeof(unsigned int) * model->indicies_count);
    memcpy((char *)serialized.data + sizeof(bool) + sizeof(int) + sizeof(vec3) * model->verticies_count + sizeof(int) + sizeof(unsigned int) * model->indicies_count, (char *)&model->uv_count, sizeof(int));
    memcpy((char *)serialized.data + sizeof(bool) + sizeof(int) + sizeof(vec3) * model->verticies_count + sizeof(int) + sizeof(unsigned int) * model->indicies_count + sizeof(vec3) * model->uv_count, (char *)model->uvs, sizeof(vec3) * model->uv_count);
    memcpy((char *)serialized.data + sizeof(bool) + sizeof(int) + sizeof(vec3) * model->verticies_count + sizeof(int) + sizeof(unsigned int) * model->indicies_count + sizeof(vec3) * model->uv_count + sizeof(vec3), (char *)model->color, sizeof(vec3));

    return serialized;
}

static Model *Deserialize(SerializedDerived serialized)
{
    puts("Deserializing model");

    Model *model = malloc(sizeof(Model));
    if (!model)
    {
        ERROR_EXIT("Couldn't allocate memory for deserialized model!\n");
    }

    // Create a pointer to track our current position in the serialized data
    char *ptr = serialized.data;

    memcpy((char *)&(model->is_valid), ptr, sizeof(bool));
    ptr += sizeof(bool);

    // printf("Deserialized model, is_valid: %i\n", model->is_valid);

    memcpy(&(model->verticies_count), ptr, sizeof(int));
    ptr += sizeof(int);

    // printf("Deserialized model, vert count: %i\n", model->verticies_count);

    model->verticies = malloc(sizeof(vec3) * model->verticies_count);
    memcpy(model->verticies, ptr, sizeof(vec3) * model->verticies_count);
    ptr += sizeof(vec3) * model->verticies_count;

    memcpy(&(model->indicies_count), ptr, sizeof(int));
    ptr += sizeof(int);

    model->indicies = malloc(sizeof(unsigned int) * model->indicies_count);
    memcpy(model->indicies, ptr, sizeof(unsigned int) * model->indicies_count);
    ptr += sizeof(unsigned int) * model->indicies_count;

    memcpy(&(model->uv_count), ptr, sizeof(int));
    ptr += sizeof(int);

    model->uvs = malloc(sizeof(vec3) * model->uv_count);
    memcpy(model->uvs, ptr, sizeof(vec3) * model->uv_count);
    ptr += sizeof(vec3) * model->uv_count;

    memcpy(&(model->color), ptr, sizeof(vec4));
    ptr += sizeof(vec4);

    // printf("Deserialized model, vert count: %i, indicies count: %i, uv count: %i\n", model->verticies_count, model->indicies_count, model->uv_count);

    return model;
}

struct AModel AModel[1] = {{
    Init,
    Delete,
    InitBox,
    InitMesh,
    Load,
    Serialize,
    Deserialize,
}};