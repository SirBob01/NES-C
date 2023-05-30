#ifndef ROM_H
#define ROM_H

#include <stdbool.h>
#include <stdio.h>

#include "./memory.h"

/**
 * @brief ROM format type.
 *
 */
typedef enum {
    NES_INVALID,
    NES_1,
    NES_2,
} rom_type_t;

/**
 * @brief ROM header.
 *
 */
typedef struct {
    /**
     * @brief ROM format type.
     *
     */
    rom_type_t type;

    /**
     * @brief Size of the program ROM in bytes.
     *
     */
    unsigned long prg_rom_size;

    /**
     * @brief Size of the character ROM in bytes.
     *
     */
    unsigned long chr_rom_size;
} rom_header_t;

typedef struct {
    /**
     * @brief Data buffer.
     *
     */
    memory_t data;

    /**
     * @brief ROM metadata.
     *
     */
    rom_header_t header;
} rom_t;

/**
 * @brief Load an NES ROM file.
 *
 * @param path
 * @return rom_t
 */
rom_t load_rom(char *path);

/**
 * @brief Get the header of the ROM.
 *
 * @param buffer
 * @return rom_header_t
 */
rom_header_t get_rom_header(const char *buffer);

/**
 * @brief Free the ROM.
 *
 * @param rom
 */
void free_rom(rom_t *rom);

#endif