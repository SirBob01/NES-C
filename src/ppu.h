#ifndef PPU_H
#define PPU_H

#include "./display.h"
#include "./interrupt.h"
#include "./memory.h"
#include "./ppu_bus.h"
#include "./rom.h"

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
     * @brief Number of cycles.
     *
     */
    unsigned long cycles;

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
     * @brief Pointer to the PPU bus.
     *
     */
    ppu_bus_t *bus;

    /**
     * @brief Pointer to the interrupt state.
     *
     */
    interrupt_t *interrupt;
} ppu_t;

/**
 * @brief Create the PPU.
 *
 * @param ppu
 * @param bus
 * @param interrupt
 */
void create_ppu(ppu_t *ppu, ppu_bus_t *bus, interrupt_t *interrupt);

/**
 * @brief Destroy the PPU.
 *
 * @param ppu
 */
void destroy_ppu(ppu_t *ppu);

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