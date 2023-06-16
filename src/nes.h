#ifndef NES_H
#define NES_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./emulator.h"
#include "./io.h"

#define ARG_INPUT_FILE "-i"

/**
 * @brief Print usage (help) information.
 *
 */
void print_usage();

/**
 * @brief Print ROM information.
 *
 * @param rom
 */
void print_rom(rom_t *rom);

/**
 * @brief Parse command line arguments to boot up the emulator.
 *
 * @param argc
 * @param argv
 * @return emulator_t
 */
emulator_t parse_args(int argc, char **argv);

#endif