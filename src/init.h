#ifndef INIT_H
#define INIT_H

#define ARG_INPUT_FILE "-i"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief ROM file data
 *
 */
typedef struct {
    /**
     * @brief Heap allocated memory containing byte code
     *
     */
    char *buffer;

    /**
     * @brief Size of the buffer
     *
     */
    unsigned long size;
} rom_t;

/**
 * @brief Print the usage of the program.
 *
 */
void print_usage();

/**
 * @brief Parse the program arguments and load ROM byte code into memory.
 *
 * @param argc
 * @param argv
 * @return rom_t
 */
rom_t parse_args(int argc, char **argv);

/**
 * @brief Free the allocated ROM.
 *
 * @param rom
 */
void free_rom(rom_t *rom);

#endif