#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Memory structure.
 *
 */
typedef struct {
    /**
     * @brief Heap allocated buffer.
     *
     */
    unsigned char *buffer;

    /**
     * @brief Size of the buffer.
     *
     */
    unsigned long size;
} memory_t;

/**
 * @brief Address type.
 *
 */
typedef unsigned short address_t;

/**
 * @brief Allocate memory.
 *
 * @param size
 * @return memory_t
 */
memory_t allocate_memory(unsigned long size);

/**
 * @brief Free allocated memory.
 *
 * @param memory
 */
void free_memory(memory_t *memory);

#endif