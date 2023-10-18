#ifndef ROM_H
#define ROM_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
     * @brief Size of the trainer segment (512-bytes or 0).
     *
     */
    unsigned trainer_size;

    /**
     * @brief Size of the program ROM in bytes.
     *
     */
    unsigned prg_rom_size;

    /**
     * @brief Size of the program RAM in bytes.
     *
     */
    unsigned prg_ram_size;

    /**
     * @brief Size of the character ROM in bytes.
     *
     */
    unsigned chr_rom_size;

    /**
     * @brief Size of the character RAM in bytes.
     *
     */
    unsigned chr_ram_size;

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
     * @brief ROM metadata.
     *
     */
    rom_header_t header;

    /**
     * @brief Data buffer.
     *
     */
    memory_t data;
} rom_t;

/**
 * @brief Load an NES ROM file.
 *
 * @param rom
 * @param path
 */
void load_rom(rom_t *rom, const char *path);

/**
 * @brief Release the ROM.
 *
 * @param rom
 */
void unload_rom(rom_t *rom);

/**
 * @brief Get the header of the ROM.
 *
 * @param buffer
 * @return rom_header_t
 */
rom_header_t get_header_rom(const unsigned char *buffer);

/**
 * @brief Get the pointer to the trainer.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_trainer_rom(rom_t *rom);

/**
 * @brief Get the pointer to PRG ROM.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_prg_rom(rom_t *rom);

/**
 * @brief Get the pointer to PRG RAM.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_prg_ram(rom_t *rom);

/**
 * @brief Get the pointer to CHR ROM.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_chr_rom(rom_t *rom);

/**
 * @brief Get the pointer to CHR RAM.
 *
 * @param rom
 * @return unsigned char*
 */
unsigned char *get_chr_ram(rom_t *rom);

/**
 * @brief Read the current state of the cartridge ROM for debugging.
 *
 * @param rom
 * @param buffer
 * @param buffer_size
 */
void read_state_rom(rom_t *rom, char *buffer, unsigned buffer_size);

#endif