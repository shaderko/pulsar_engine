/**
 * @file scene.c
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-05-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>

#include "../../util/util.h"
#include "scene.h"
#include "../object.h"

static Scene *Init(vec3 *size)
{
    Scene *scene = malloc(sizeof(Scene));
    if (!scene)
    {
        ERROR_EXIT("Scene memory couldn't be allocated!\n");
    }

    memcpy(scene->size, size, sizeof(vec3));
    scene->objects = NULL;
    scene->objects_size = 0;

    return scene;
}

static void Update(Scene *scene)
{
    for (int i = 0; i < scene->objects_size; i++)
    {
        AObject.Update(scene->objects[i]);
    }
}

static void Add(Scene *scene, Object *object)
{
    if (!scene || !object)
    {
        return;
    }
    scene->objects = realloc(scene->objects, sizeof(Object *) * (scene->objects_size + 1));
    scene->objects[scene->objects_size] = object;
    scene->objects_size++;
}

static void WriteToFile(Scene *scene, const char *file)
{
    FILE *out = fopen(file, "wb");
    if (out == NULL)
    {
        printf("Error opening file: %s\n", file);
        return;
    }

    // Write the total number of objects
    fwrite(&scene->objects_size, sizeof(int), 1, out);

    // Write objects data
    for (int i = 0; i < scene->objects_size; i++)
    {
        Object *object = scene->objects[i];

        SerializedDerived serialized_object = AObject.Serialize(object);
        fwrite(serialized_object.data, serialized_object.len, 1, out);
    }

    fclose(out);
}

static void ReadFile(Scene *scene, const char *file)
{
    FILE *in = fopen(file, "rb");
    if (!in)
    {
        printf("Error opening file: %s\n", file);
        return;
    }

    int size;
    fread(&size, sizeof(int), 1, in);

    // Read objects data
    for (int i = 0; i < size; i++)
    {
        SerializedObject *object = malloc(sizeof(SerializedObject));
        fread(object, sizeof(SerializedObject), 1, in);

        // Collider
        object->collider.derived.data = malloc(object->collider.derived.len);
        fread(object->collider.derived.data, object->collider.derived.len, 1, in);

        // Renderer
        object->renderer.derived.data = malloc(object->renderer.derived.len);
        fread(object->renderer.derived.data, object->renderer.derived.len, 1, in);

        AScene->Add(scene, AObject.Deserialize(object, scene));

        free(object->collider.derived.data);
        free(object->renderer.derived.data);
        free(object);
    }
    fclose(in);
}

// Load map
// Object *object = AObject.InitBox(false, 1, (vec3){100, 400, 0}, (vec3){100, 100, 100});
// Object *object1 = AObject.InitBox(true, 1, (vec3){100, 100, 0}, (vec3){300, 100, 100});
// Object *object2 = AObject.InitBox(false, 1, (vec3){100, 100, 0}, (vec3){100, 100, 100});
// AScene->Add(room->scene, object);
// AScene->Add(room->scene, object1);
// AScene->Add(room->scene, object2);
// AScene->WriteToFile(room->scene, "file");

struct AScene AScene[1] =
    {{
        Init,
        Update,
        Add,
        WriteToFile,
        ReadFile,
    }};