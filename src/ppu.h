#ifndef PPU_H
#define PPU_H

#include "./display.h"
#include "./memory.h"

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
#define PPU_SCANLINES  262
#define PPU_SCANCYCLES 341

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
     * @brief Output color buffer.
     *
     */
    color_t color_buffer[PPU_SCANCYCLES * PPU_SCANLINES];
} ppu_t;

/**
 * @brief Create the PPU.
 *
 * @return ppu_t*
 */
ppu_t *create_ppu();

/**
 * @brief Destroy the PPU.
 *
 * @param ppu
 */
void destroy_ppu(ppu_t *ppu);

/**
 * @brief Update the PPU.
 *
 * @param ppu
 */
void update_ppu(ppu_t *ppu);

#endif