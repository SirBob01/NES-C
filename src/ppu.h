#ifndef PPU_H
#define PPU_H

#include "./display.h"
#include "./interrupt.h"
#include "./memory.h"
#include "./rom.h"

// PPU has a 16-bit address bus (64k)
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

// Rows and columns of the PPU render target
#define PPU_SCANLINES 262
#define PPU_LINEDOTS  341

typedef struct {
    /**
     * @brief PPUCTRL register.
     *
     */
    unsigned char ctrl;

    /**
     * @brief PPUMASK register.
     *
     */
    unsigned char mask;

    /**
     * @brief PPUSTATUS register.
     *
     */
    unsigned char status;

    /**
     * @brief OAMADDR register.
     *
     */
    unsigned char oam_addr;

    /**
     * @brief OAMDATA register.
     *
     */
    unsigned char oam_data;

    /**
     * @brief PPUSCROLL register.
     *
     */
    unsigned char scroll;

    /**
     * @brief PPUADDR register.
     *
     */
    unsigned char addr;

    /**
     * @brief PPUDATA register.
     *
     */
    unsigned char data;

    /**
     * @brief OAMDMA register.
     *
     */
    unsigned char oam_dma;

    /**
     * @brief Current scanline (row).
     *
     */
    unsigned scanline;

    /**
     * @brief Current pixel (column).
     *
     */
    unsigned dot;

    /**
     * @brief Is in an odd frame?
     *
     */
    bool odd_frame;

    /**
     * @brief PPU video memory.
     *
     */
    memory_t memory;

    /**
     * @brief Object attribute memory.
     *
     */
    unsigned char oam[256];

    /**
     * @brief Output color buffer.
     *
     */
    color_t color_buffer[PPU_LINEDOTS * PPU_SCANLINES];

    /**
     * @brief Pointer to the CPU interrupt controller.
     *
     */
    interrupt_t *interrupt;

    /**
     * @brief Pointer to the ROM.
     *
     */
    rom_t *rom;
} ppu_t;

/**
 * @brief Create the PPU.
 *
 * @param rom
 * @return ppu_t*
 */
ppu_t *create_ppu(rom_t *rom);

/**
 * @brief Destroy the PPU.
 *
 * @param ppu
 */
void destroy_ppu(ppu_t *ppu);

/**
 * @brief Convert mirrored addresses to actual addresses.
 *
 * @param address
 * @param mirroring
 * @return address_t
 */
address_t mirror_address_ppu(address_t address, rom_mirroring_t mirroring);

/**
 * @brief Apply mapper to address that lies on the CHR cartridge section.
 *
 * @param ppu
 * @param address
 * @return unsigned char*
 */
unsigned char *apply_memory_mapper_ppu(ppu_t *cpu, address_t address);

/**
 * @brief Get the pointer to an address in VRAM.
 *
 * @param ppu
 * @param address
 * @return unsigned char*
 */
unsigned char *get_memory_ppu(ppu_t *ppu, address_t address);

/**
 * @brief Read the current state of the CPU for debugging.
 *
 * @param ppu
 * @param buffer
 * @param buffer_size
 */
void read_state_ppu(ppu_t *ppu, char *buffer, unsigned buffer_size);

/**
 * @brief Update the PPU.
 *
 * @param ppu
 */
void update_ppu(ppu_t *ppu);

#endif