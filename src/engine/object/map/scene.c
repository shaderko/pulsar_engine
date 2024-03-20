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
    scene->objects_list = NULL;
    scene->objects_list_size = 0;

    return scene;
}

static void Delete(Scene *scene)
{
    // for (int i = 0; i < scene->objects_size; i++)
    // {
    //     AObject.Delete(scene->objects[i]);
    // }

    // free(scene->objects);
    // free(scene);
}

static void Update(Scene *scene)
{
    // for (int i = 0; i < scene->objects_size; i++)
    // {
    //     AObject.Update(scene->objects[i]);
    // }
}

static void Add(Scene *scene, Object *object)
{
    if (!scene || !object)
    {
        return;
    }

    for (int i = 0; i < scene->objects_list_size; i++)
    {
        // Compare model->verticies and indicies to see if they are the same
        // for (int v = 0; v < scene->objects_list[i]->object[0]->renderer->model->verticies_count; v++)
        // {

        // }

        if (scene->objects_list[i]->object[0]->renderer->model->verticies_count == object->renderer->model->verticies_count)
        {
            printf("Object with model, already exists in scene, adding to array\n");

            scene->objects_list[i]->object = realloc(scene->objects_list[i]->object, sizeof(Object *) * (scene->objects_list[i]->object_size + 1));
            scene->objects_list[i]->object[scene->objects_list[i]->object_size] = object;
            scene->objects_list[i]->object_size++;

            return;
        }
    }

    puts("Object with model, doesn't exist in scene, creating new array\n");

    ObjectList *object_list = malloc(sizeof(ObjectList));
    object_list->object = malloc(sizeof(Object *));
    object_list->object[0] = object;
    object_list->object_size = 1;

    scene->objects_list = realloc(scene->objects_list, sizeof(ObjectList *) * (scene->objects_list_size + 1));
    scene->objects_list[scene->objects_list_size] = object_list;
    scene->objects_list_size++;
}

static void Render(Scene *scene)
{
    for (int i = 0; i < scene->objects_list_size; i++)
    {
        AObject.BatchRender(scene->objects_list[i], scene->objects_list[i]->object_size);
    }
}

static void WriteToFile(Scene *scene, const char *file)
{
    // FILE *out = fopen(file, "wb");
    // if (out == NULL)
    // {
    //     printf("Error opening file: %s\n", file);
    //     return;
    // }

    // // Write the total number of objects
    // fwrite(&scene->objects_size, sizeof(int), 1, out);

    // // Write objects data
    // for (int i = 0; i < scene->objects_size; i++)
    // {
    //     Object *object = scene->objects[i];

    //     SerializedDerived serialized_object = AObject.Serialize(object);
    //     fwrite(serialized_object.data, serialized_object.len, 1, out);
    // }

    // fclose(out);
}

static void ReadFile(Scene *scene, const char *file)
{
    // FILE *in = fopen(file, "rb");
    // if (!in)
    // {
    //     printf("Error opening file: %s\n", file);
    //     return;
    // }

    // int size;
    // fread(&size, sizeof(int), 1, in);

    // // Read objects data
    // for (int i = 0; i < size; i++)
    // {
    //     SerializedObject *object = malloc(sizeof(SerializedObject));
    //     fread(object, sizeof(SerializedObject), 1, in);

    //     // Collider
    //     object->collider.derived.data = malloc(object->collider.derived.len);
    //     fread(object->collider.derived.data, object->collider.derived.len, 1, in);

    //     // Renderer
    //     object->renderer.derived.data = malloc(object->renderer.derived.len);
    //     fread(object->renderer.derived.data, object->renderer.derived.len, 1, in);

    //     AScene->Add(scene, AObject.Deserialize(object, scene));

    //     free(object->collider.derived.data);
    //     free(object->renderer.derived.data);
    //     free(object);
    // }
    // fclose(in);
}

struct AScene AScene[1] =
    {{
        Init,
        Delete,
        Update,
        Add,
        Render,
        WriteToFile,
        ReadFile,
    }};