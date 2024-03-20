/**
 * @file scene.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-05-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SCENE_H
#define SCENE_H

#include <linmath.h>

#include "../object.h"

typedef struct ObjectList ObjectList;
struct ObjectList
{
    Object **object;
    int object_size;
};

typedef struct Scene Scene;
struct Scene
{
    /**
     * Defines bounds of a scene
     */
    vec3 size;

    /**
     * Array of objects in a scene
     */
    ObjectList **objects_list;
    int objects_list_size;
};

struct AScene
{
    /**
     * Initializes a scene
     */
    Scene *(*Init)(vec3 *);
    void (*Delete)(Scene *scene);

    /**
     * Updates a scene
     */
    void (*Update)(Scene *scene);

    /**
     * Adds an object to a scene
     */
    void (*Add)(Scene *scene, Object *object);

    void (*Render)(Scene *scene);

    /**
     * Writes scene objects to a file
     */
    void (*WriteToFile)(Scene *scene, const char *file);

    /**
     * Reads scene objects from a file
     */
    void (*ReadFile)(Scene *scene, const char *file);
};

extern struct AScene AScene[1];

#endif