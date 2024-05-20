/**
 * @file thread.c
 * @author your name (you@domain.com)
 * @brief Single thread
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "thread.h"
#include "../../util/util.h"

// Create, delete and update thread methods

static Thread *Init(void *(*func)(void *), void *data)
{
    Thread *thread = malloc(sizeof(Thread));
    if (!thread)
        ERROR_EXIT("[ERROR] Thread memory couldn't be allocated.");
}

static void Delete(Thread *thread)
{
    free(thread);

    return;
}

struct AThread AThread =
    {
        .Init = Init,
        .Delete = Delete,
};