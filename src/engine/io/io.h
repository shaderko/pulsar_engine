/**
 * @file io.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-07-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef IO_H
#define IO_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct file
{
    char *data;
    size_t len;
    bool is_valid;
} File;

File io_file_read(const char *path);
int io_file_write(void *buffer, size_t size, const char *path);

#endif