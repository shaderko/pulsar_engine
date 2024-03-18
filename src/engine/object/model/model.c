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
#include <time.h>
#include <SDL.h>

#include "../../util/util.h"

MAX_BUFFER_SIZE = 65536;

// Init function should be called to load from cache
static Model *
Init()
{
    Model *model = malloc(sizeof(Model));

    // Initialize model to zero/null
    memset(model, 0, sizeof(Model));
    model->is_valid = false;

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
    // memcpy(model->verticies[0], (vec3){-1, -1, -1}, sizeof(vec3));
    // memcpy(model->verticies[1], (vec3){1, -1, -1}, sizeof(vec3));
    // memcpy(model->verticies[2], (vec3){1, 1, -1}, sizeof(vec3));
    // memcpy(model->verticies[3], (vec3){-1, 1, -1}, sizeof(vec3));
    // memcpy(model->verticies[4], (vec3){-1, -1, 1}, sizeof(vec3));
    // memcpy(model->verticies[5], (vec3){1, -1, 1}, sizeof(vec3));
    // memcpy(model->verticies[6], (vec3){1, 1, 1}, sizeof(vec3));
    // memcpy(model->verticies[7], (vec3){-1, 1, 1}, sizeof(vec3));

    model->indicies_count = 36;
    model->indicies = malloc(sizeof(unsigned int) * model->indicies_count);
    if (!model->indicies)
    {
        ERROR_EXIT("Couldn't allocate memory for indicies!\n");
    }

    model->uv_count = 0;
    model->uvs = NULL;

    unsigned int indicesArray[] = {0, 1, 2, 0, 2, 3, 1, 5, 6, 1, 6, 2, 5, 4, 7, 5, 7, 6, 4, 0, 3, 4, 3, 7, 3, 2, 6, 3, 6, 7, 0, 1, 5, 0, 5, 4};
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
        ERROR_RETURN(NULL, "Failed to open file: %s\n", path);

    Model *model = AModel->Init();

    clock_t start, end;
    start = clock();

    // Allocate space for verticies ahead of time
    unsigned int current_verticies_size = MAX_BUFFER_SIZE;
    model->verticies = malloc(sizeof(float) * current_verticies_size);
    if (!model->verticies)
        ERROR_EXIT("Failed to allocate memory for vertices\n");

    // Allocate space for indicies ahead of time
    unsigned int current_indicies_size = MAX_BUFFER_SIZE;
    model->indicies = malloc(sizeof(unsigned int) * current_indicies_size);
    if (!model->indicies)
        ERROR_EXIT("Failed to allocate memory for indicies\n");

    char line[128];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (line[0] == 'v' && line[1] == ' ')
        {
            // Vertices
            if (current_verticies_size <= model->verticies_count + 3)
            {
                // Allocate more space for verticies
                current_verticies_size += MAX_BUFFER_SIZE;
                model->verticies = realloc(model->verticies, sizeof(float) * current_verticies_size);
                if (!model->verticies)
                    ERROR_EXIT("Failed to allocate memory for vertices\n");
            }

            // Read vertex coordinates directly into allocated space
            char *ptr = &line[2]; // Start parsing after "v "
            for (int i = 0; i < 3; i++)
            {
                model->verticies[model->verticies_count++] = strtof(ptr, &ptr);
            }
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            if (current_indicies_size <= model->indicies_count + 3)
            {
                // Allocate more space for verticies
                current_indicies_size += MAX_BUFFER_SIZE;
                model->indicies = realloc(model->indicies, sizeof(unsigned int) * current_indicies_size);
                if (!model->indicies)
                    ERROR_EXIT("Failed to allocate memory for indicies\n");
            }

            char *ptr = &line[2]; // Start parsing after "f "
            long val;
            for (int i = 0; i < 3; i++)
            {
                errno = 0;
                val = strtol(ptr, &ptr, 10); // Parse integer
                if (errno != 0)
                {
                    // Handle potential error
                    break;
                }
                model->indicies[model->indicies_count++] = (unsigned int)val - 1; // Adjust indices to be 0-based
                while (*ptr && *ptr != ' ' && *ptr != '\n')
                    ++ptr;
            }
        }
    }

    realloc(model->verticies, sizeof(float) * model->verticies_count);
    if (!model->verticies)
        ERROR_EXIT("Failed to allocate memory for vertices\n");
    realloc(model->indicies, sizeof(unsigned int) * model->indicies_count);
    if (!model->indicies)
        ERROR_EXIT("Failed to allocate memory for indicies\n");

    fclose(file);
    model->is_valid = true;

    end = clock(); // End timer
    double time_taken = ((double)(end - start)) * 1000.0 / CLOCKS_PER_SEC;

    printf("Model loaded successfully in %.2fms\nVert count: %u, indices count: %u, uv count: %u\n", time_taken, model->verticies_count, model->indicies_count, model->uv_count);
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