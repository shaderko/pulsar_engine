/**
 * @file threads_manager.c
 * @author your name (you@domain.com)
 * @brief Manages all threads and spreads the work load
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "threads_manager.h"

#define MAX_THREADS 254

// Create all threads and dynamically adjust by the amount of work