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
 * @brief ROM mirroring type.
 *
 */
typedef enum {
    MIRROR_HORIZONTAL,
    MIRROR_VERTICAL,
} rom_mirroring_t;

/**
 * @brief Nintendo console type.
 *
 */
typedef enum {
    CONSOLE_NES,
    CONSOLE_VS,
    CONSOLE_PLAYCHOICE,
    CONSOLE_EXTENDED,
} rom_console_type_t;

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

    /**
     * @brief ROM mirroring mode.
     *
     */
    rom_mirroring_t mirroring;

    /**
     * @brief Existence of non-volatile memory.
     *
     */
    bool battery;

    /**
     * @brief 512-byte trainer segment exists.
     *
     */
    bool trainer;

    /**
     * @brief Hardwired four-screen VRAM.
     *
     */
    bool four_screen;

    /**
     * @brief Mapper number.
     *
     */
    unsigned short mapper;

    /**
     * @brief Console type.
     *
     */
    rom_console_type_t console_type;
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
rom_t load_rom(const char *path);

/**
 * @brief Get the header of the ROM.
 *
 * @param buffer
 * @return rom_header_t
 */
rom_header_t get_rom_header(const unsigned char *buffer);

/**
 * @brief Get the pointer to the trainer.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_trainer(rom_t *rom);

/**
 * @brief Get the pointer to PRG ROM.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_prg_rom(rom_t *rom);

/**
 * @brief Get the pointer to CHR ROM.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_chr_rom(rom_t *rom);

/**
 * @brief Release the ROM.
 *
 * @param rom
 */
void unload_rom(rom_t *rom);

#endif