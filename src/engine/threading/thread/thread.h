/**
 * @file thread.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>
#include "../../util/util.h"

typedef struct Thread Thread;
struct Thread
{
    void *(*func)(void *);
    void *data;

    bool done;
};

struct AThread
{
    Thread *(*Init)(void *(*func)(void *), void *data);
    void (*Delete)(Thread *thread);
};

extern struct AScene AScene;

#endif