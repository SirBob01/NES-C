#ifndef PPU_BUS_H
#define PPU_BUS_H

#include "./memory.h"
#include "./rom.h"

// 2C02 has a 16-bit address bus (64k)
#define PPU_RAM_SIZE 1 << 16

// PPU memory map address offsets
#define PPU_MAP_PATTERNTABLE_0   0x0000
#define PPU_MAP_PATTERNTABLE_1   0x1000
#define PPU_MAP_NAMETABLE_0      0x2000
#define PPU_MAP_NAMETABLE_1      0x2400
#define PPU_MAP_NAMETABLE_2      0x2800
#define PPU_MAP_NAMETABLE_3      0x2c00
#define PPU_MAP_NAMETABLE_MIRROR 0x3000
#define PPU_MAP_PALETTE          0x3f00
#define PPU_MAP_PALETTE_MIRROR   0x3f20

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
} ppu_bus_t;

address_t mirror_address_ppu_bus(address_t address, rom_mirroring_t mirroring);
unsigned char *apply_memory_mapper_ppu_bus(ppu_bus_t *bus, address_t address);
unsigned char *get_memory_ppu_bus(ppu_bus_t *bus, address_t address);

/**
 * @brief Create the PPU bus.
 *
 * @param bus
 * @param rom
 */
void create_ppu_bus(ppu_bus_t *bus, rom_t *rom);

/**
 * @brief Read a byte from the PPU address bus.
 *
 * @param bus
 * @param addr
 * @return unsigned char
 */
unsigned char read_ppu_bus(ppu_bus_t *bus, address_t addr);

/**
 * @brief Write a byte to the PPU address bus.
 *
 * @param bus
 * @param addr
 * @param data
 */
void write_ppu_bus(ppu_bus_t *bus, address_t addr, unsigned char data);

#endif