/**
 * @file terminal.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-06-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>

typedef Terminal Terminal;
struct Terminal
{
    bool show;

    float width;
    float height;

    float x;
    float y;

    float padding;
    float text_size;

    char *text;
};

struct ATerminal
{
    Terminal *(*Init)(void);
    void (*Draw)(Terminal *terminal);

    int (*AddCommand)(Terminal *terminal, char *command, void (*callback)(void));
    int (*RemoveCommand)(Terminal *terminal, char *command);
};

extern struct ATerminal ATerminal[1];

#endif