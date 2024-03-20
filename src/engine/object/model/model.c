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
#include <glad/glad.h>
#include <SDL.h>

#include "../../util/util.h"

MAX_CHUNK_SIZE = (1024 * 1024);
MAX_BUFFER_SIZE = 65536;

static Model *Init()
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

    glDeleteVertexArrays(1, &model->vao);
    glDeleteBuffers(1, &model->vbo);
    glDeleteBuffers(1, &model->ebo);

    free(model);
}

static Model *InitBox()
{
    Model *model = AModel->Init();

    model->verticies_count = 24;
    model->verticies = malloc(sizeof(float) * model->verticies_count);
    float verticies[] = {-1, -1, -1,
                         1, -1, -1,
                         1, 1, -1,
                         -1, 1, -1,
                         -1, -1, 1,
                         1, -1, 1,
                         1, 1, 1,
                         -1, 1, 1};
    memcpy(model->verticies, verticies, sizeof(float) * model->verticies_count);

    model->indicies_count = 36;
    model->indicies = malloc(sizeof(unsigned int) * model->indicies_count);
    if (!model->indicies)
        ERROR_EXIT("Couldn't allocate memory for indicies!\n");

    model->uv_count = 0;
    model->uvs = NULL;

    unsigned int indicesArray[] = {0, 1, 2, 0, 2, 3, 1, 5, 6, 1, 6, 2, 5, 4, 7, 5, 7, 6, 4, 0, 3, 4, 3, 7, 3, 2, 6, 3, 6, 7, 0, 1, 5, 0, 5, 4};
    memcpy(model->indicies, indicesArray, sizeof(unsigned int) * model->indicies_count);

    model->is_valid = true;

    // Upload the data to the GPU so it can be reused
    glGenVertexArrays(1, &model->vao);
    glGenBuffers(1, &model->vbo);
    glGenBuffers(1, &model->ebo);

    glBindVertexArray(model->vao);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);

    glBufferData(GL_ARRAY_BUFFER, model->verticies_count * sizeof(float), model->verticies, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indicies_count * sizeof(unsigned int), model->indicies, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

typedef struct model_load_helper_args
{
    char *chunk;
    size_t chunk_size;

    unsigned int current_verticies_size;
    unsigned int current_indicies_size;

    unsigned int verticies_count;
    unsigned int indicies_count;

    float *verticies;
    unsigned int *indicies;
} model_load_helper_args;

static void process_line(char *line, model_load_helper_args *args)
{
    if (line[0] == 'v' && line[1] == ' ')
    {
        // Vertices
        if (args->current_verticies_size <= args->verticies_count + 3)
        {
            // Allocate more space for verticies
            args->current_verticies_size += MAX_BUFFER_SIZE;
            args->verticies = realloc(args->verticies, sizeof(float) * args->current_verticies_size);
            if (!args->verticies)
                ERROR_EXIT("Failed to allocate memory for vertices\n");
        }

        // Read vertex coordinates directly into allocated space
        char *ptr = &line[2]; // Start parsing after "v "
        for (int i = 0; i < 3; i++)
        {
            args->verticies[args->verticies_count++] = strtof(ptr, &ptr);
        }
    }
    else if (line[0] == 'f' && line[1] == ' ')
    {
        if (args->current_indicies_size <= args->indicies_count + 3)
        {
            // Allocate more space for verticies
            args->current_indicies_size += MAX_BUFFER_SIZE;
            args->indicies = realloc(args->indicies, sizeof(unsigned int) * args->current_indicies_size);
            if (!args->indicies)
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
            args->indicies[args->indicies_count++] = (unsigned int)val - 1; // Adjust indices to be 0-based
            while (*ptr && *ptr != ' ' && *ptr != '\n')
                ++ptr;
        }
    }
}

static void model_load_helper(void *args)
{
    model_load_helper_args *helper_args = (model_load_helper_args *)args;

    // Allocate space for verticies and indicies
    helper_args->verticies = malloc(sizeof(float) * MAX_BUFFER_SIZE);
    if (!helper_args->verticies)
        ERROR_EXIT("Failed to allocate memory for vertices\n");
    helper_args->current_verticies_size = MAX_BUFFER_SIZE;
    helper_args->verticies_count = 0;

    helper_args->indicies = malloc(sizeof(unsigned int) * MAX_BUFFER_SIZE);
    if (!helper_args->indicies)
        ERROR_EXIT("Failed to allocate memory for indicies\n");
    helper_args->current_indicies_size = MAX_BUFFER_SIZE;
    helper_args->indicies_count = 0;

    char *ptr = helper_args->chunk;
    char *line_start = ptr; // Pointer to the start of the current line
    while (*ptr != '\0')
    { // Iterate until the end of the chunk
        if (*ptr == '\n' || *ptr == '\r')
        {
            *ptr = '\0'; // Null-terminate the current line for processing
            process_line(line_start, helper_args);

            ptr++; // Move past the newline character
            // Handle \r\n (Windows) newline sequences
            if (*ptr == '\n')
            {
                ptr++;
            }
            line_start = ptr; // Set the start of the next line
        }
        else
        {
            ptr++; // Move to the next character
        }
    }

    // Process any remaining line if the file doesn't end with a newline
    if (ptr != line_start)
    {
        process_line(line_start, helper_args);
    }
}

static Model *Load(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
        ERROR_RETURN(NULL, "Failed to open file: %s\n", path);

    Model *model = AModel->Init();

    clock_t start, end;
    start = clock();

    char *chunk = calloc(1, MAX_CHUNK_SIZE);
    if (!chunk)
        ERROR_EXIT("Failed to allocate memory for file chunk\n");

    size_t num_of_threads = 0;
    SDL_Thread **threadIds = NULL;
    model_load_helper_args **args_list = NULL;

    // Read the file in chunks to avoid using too much memory
    size_t bytes_read;
    while ((bytes_read = fread(chunk, sizeof(char), MAX_CHUNK_SIZE, file)) > 0)
    {
        // Separate each chunk processing into a thread
        model_load_helper_args *args = malloc(sizeof(model_load_helper_args));

        args->chunk = malloc(bytes_read);
        memcpy(args->chunk, chunk, bytes_read);
        args->chunk_size = bytes_read;

        SDL_Thread *threadId = SDL_CreateThread(model_load_helper, "model_load_helper", (void *)args);

        threadIds = realloc(threadIds, sizeof(SDL_Thread *) * (num_of_threads + 1));
        args_list = realloc(args_list, sizeof(model_load_helper_args *) * (num_of_threads + 1));
        args_list[num_of_threads] = args;
        threadIds[num_of_threads] = threadId;
        num_of_threads++;
    }

    // Wait for all threads to finish
    for (size_t i = 0; i < num_of_threads; i++)
    {
        SDL_WaitThread(threadIds[i], NULL);
    }

    unsigned int total_verticies = 0;
    unsigned int total_indicies = 0;

    // Count all verticies and indicies so we can allocate the correct amount of memory
    for (size_t i = 0; i < num_of_threads; i++)
    {
        total_verticies += args_list[i]->verticies_count;
        total_indicies += args_list[i]->indicies_count;
    }

    // Allocate the mentioned memory
    model->verticies = malloc(sizeof(float) * total_verticies);
    if (!model->verticies)
        ERROR_EXIT("Failed to allocate memory for model verticies\n");
    model->verticies_count = total_verticies;

    model->indicies = malloc(sizeof(unsigned int) * total_indicies);
    if (!model->indicies)
        ERROR_EXIT("Failed to allocate memory for model indicies\n");
    model->indicies_count = total_indicies;

    unsigned int current_verticies_count = 0;
    unsigned int current_indicies_count = 0;

    // Merge all the verticies and indicies together and free each thread's memory
    for (size_t i = 0; i < num_of_threads; i++)
    {
        model_load_helper_args *arg = args_list[i];

        memcpy(&model->verticies[current_verticies_count], arg->verticies, sizeof(float) * arg->verticies_count);
        memcpy(&model->indicies[current_indicies_count], arg->indicies, sizeof(unsigned int) * arg->indicies_count);

        current_verticies_count += arg->verticies_count;
        current_indicies_count += arg->indicies_count;

        free(arg->verticies);
        free(arg->indicies);
        free(arg->chunk);
        free(arg);
    }

    // Clean up
    free(chunk);
    free(threadIds);
    free(args_list);
    fclose(file);

    model->is_valid = true;

    // Upload the data to the GPU so it can be reused
    glGenVertexArrays(1, &model->vao);
    glGenBuffers(1, &model->vbo);
    glGenBuffers(1, &model->ebo);

    glBindVertexArray(model->vao);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);

    glBufferData(GL_ARRAY_BUFFER, model->verticies_count * sizeof(float), model->verticies, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indicies_count * sizeof(unsigned int), model->indicies, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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