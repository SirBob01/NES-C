#ifndef NES_H
#define NES_H

#include <assert.h>
#include <stdio.h>
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
 * @brief Parse command line arguments to boot up the emulator.
 *
 * @param emu
 * @param argc
 * @param argv
 */
void parse_args(emulator_t *emu, int argc, char **argv);

#endif