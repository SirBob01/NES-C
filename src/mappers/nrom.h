#ifndef MAPPER_NROM_H
#define MAPPER_NROM_H

#include "../memory.h"
#include "../rom.h"

/**
 * @brief Read from NROM CPU memory.
 *
 * @param rom
 * @param address
 */
unsigned char read_cpu_nrom(rom_t *rom, address_t address);

/**
 * @brief Read from NROM PPU memory.
 *
 * @param rom
 * @param address
 */
unsigned char read_ppu_nrom(rom_t *rom, address_t address);

/**
 * @brief Write to NROM CPU memory. This will throw an error.
 *
 * @param rom
 * @param address
 * @param value
 */
void write_cpu_nrom(rom_t *rom, address_t address, unsigned char value);

/**
 * @brief Write to NROM PPU memory. This will throw an error.
 *
 * @param rom
 * @param address
 * @param value
 */
void write_ppu_nrom(rom_t *rom, address_t address, unsigned char value);

#endif