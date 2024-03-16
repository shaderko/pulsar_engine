/**
 * @file util.c
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-06-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "util.h"

ull generate_random_id()
{
    static int initialized = 0;
    if (!initialized)
    {
        srand(time(NULL)); // Seed the random number generator with the current time
        initialized = 1;
    }

    // Generate a random 64-bit unsigned integer
    ull id = ((ull)rand() << 32) | rand();
    return id;
}
