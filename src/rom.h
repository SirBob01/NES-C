#ifndef ROM_H
#define ROM_H

#include <stdio.h>

#include "./memory.h"

/**
 * @brief Load an NES ROM file.
 *
 * @param path
 * @return memory_t
 */
memory_t load_rom(char *path);

#endif