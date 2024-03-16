/**
 * @file config.h
 * @author https://github.com/shaderko
 * @brief
 * @version 0.1
 * @date 2023-06-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "../../input/input.h"
#include "../types/types.h"

typedef struct config
{
    u8 keybinds[5];
} Config_State;

void config_init(void);
void config_key_bind(Input_Key key, const char *key_name);

#endif