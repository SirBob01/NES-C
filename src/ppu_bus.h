#ifndef PPU_BUS_H
#define PPU_BUS_H

#include "./mapper.h"
#include "./memory.h"
#include "./rom.h"

// 2C02 has a 16-bit address bus (64k)
#define PPU_RAM_SIZE           (1 << 16)
#define PPU_PRIMARY_OAM_SIZE   (1 << 8)
#define PPU_SECONDARY_OAM_SIZE (1 << 5)
#define PPU_PALETTE_SIZE       (1 << 5)

// PPU memory map address offsets
#define PPU_MAP_PATTERNTABLE_0   0x0000
#define PPU_MAP_PATTERNTABLE_1   0x1000
#define PPU_MAP_NAMETABLE_0      0x2000
#define PPU_MAP_NAMETABLE_1      0x2400
#define PPU_MAP_NAMETABLE_2      0x2800
#define PPU_MAP_NAMETABLE_3      0x2C00
#define PPU_MAP_NAMETABLE_MIRROR 0x3000
#define PPU_MAP_PALETTE          0x3F00
#define PPU_MAP_PALETTE_MIRROR   0x3F20

/**
 * @brief PPU bus subsystem.
 *
 */
typedef struct {
    /**
     * @brief PPU memory address space.
     *
     */
    unsigned char memory[PPU_RAM_SIZE];

    /**
     * @brief Pointer to the ROM.
     *
     */
    rom_t *rom;

    /**
     * @brief Pointer to the mapper.
     *
     */
    mapper_t *mapper;
} ppu_bus_t;

/**
 * @brief Create the PPU bus.
 *
 * @param bus
 * @param rom
 * @param mapper
 */
void create_ppu_bus(ppu_bus_t *bus, rom_t *rom, mapper_t *mapper);

/**
 * @brief Mirror an address according to the ROM mirroring mode.
 *
 * @param address
 * @param mirroring
 * @return address_t
 */
address_t mirror_address_ppu_bus(address_t address, rom_mirroring_t mirroring);

/**
 * @brief Read a byte from the PPU address bus.
 *
 * @param bus
 * @param address
 * @return unsigned char
 */
unsigned char read_ppu_bus(ppu_bus_t *bus, address_t address);

/**
 * @brief Write a byte to the PPU address bus.
 *
 * @param bus
 * @param address
 * @param value
 */
void write_ppu_bus(ppu_bus_t *bus, address_t address, unsigned char value);

#endif