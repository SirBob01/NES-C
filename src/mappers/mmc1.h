#ifndef MAPPER_MMC1_H
#define MAPPER_MMC1_H

#define MMC1_SERIAL_PORT 0x8000

#include "../memory.h"
#include "../rom.h"

/**
 * @brief MMC1 mapper state.
 *
 */
typedef struct {
    /**
     * @brief Common shift register connected to the serial port.
     *
     */
    unsigned char shift_register;

    /**
     * @brief Write counter.
     *
     */
    unsigned long write_counter;
} mmc1_t;

/**
 * @brief Create the MMC1 mapper state.
 *
 * @param mapper
 */
void create_mmc1(mmc1_t *mapper);

/**
 * @brief Destroy
 *
 * @param mapper
 */
void destroy_mmc1(mmc1_t *mapper);

/**
 * @brief Read from MMC1 CPU memory.
 *
 * @param mapper
 * @param rom
 * @param address
 * @return unsigned char
 */
unsigned char read_cpu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address);

/**
 * @brief Read from MMC1 PPU memory.
 *
 * @param mapper
 * @param rom
 * @param address
 * @return unsigned char
 */
unsigned char read_ppu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address);

/**
 * @brief Write to MMC1 CPU memory.
 *
 * @param mapper
 * @param rom
 * @param address
 * @param value
 */
void write_cpu_mmc1(mmc1_t *mapper,
                    rom_t *rom,
                    address_t address,
                    unsigned char value);

/**
 * @brief Write to MMC1 PPU memory.
 *
 * @param mapper
 * @param rom
 * @param address
 * @param value
 */
void write_ppu_mmc1(mmc1_t *mapper,
                    rom_t *rom,
                    address_t address,
                    unsigned char value);

#endif