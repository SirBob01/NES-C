#include "./ppu.h"

void create_ppu(ppu_t *ppu, ppu_bus_t *bus, interrupt_t *interrupt) {
    ppu->ctrl = 0;
    ppu->mask = 0;
    ppu->status = 0;
    ppu->oam_addr = 0;
    ppu->oam_data = 0;
    ppu->scroll = 0;
    ppu->addr = 0;
    ppu->data = 0;
    ppu->oam_dma = 0;

    ppu->cycles = 21; // Initial cycle count
    ppu->scanline = 0;
    ppu->dot = ppu->cycles;

    ppu->odd_frame = false;

    ppu->bus = bus;
    ppu->interrupt = interrupt;
}

void destroy_ppu(ppu_t *ppu) {}

void read_state_ppu(ppu_t *ppu, char *buffer, unsigned buffer_size) {
    snprintf(buffer,
             buffer_size,
             "CTRL:%02X STATUS:%02X MASK:%02X SCROLL:%02X ADDR:%02X DATA:%02X "
             "OAMADDR:%02X OAMDATA:%02X CYC:%3d,%3d",
             ppu->ctrl,
             ppu->status,
             ppu->mask,
             ppu->scroll,
             ppu->addr,
             ppu->data,
             ppu->oam_addr,
             ppu->oam_data,
             ppu->scanline,
             ppu->dot);
}

void render_ppu(ppu_t *ppu) {
    // TODO: Compute this
    unsigned palette_index = rand() % 64;

    // Write to the color buffer
    unsigned buffer_index = ppu->scanline * PPU_LINEDOTS + ppu->dot;
    ppu->color_buffer[buffer_index] = COLOR_PALETTE[palette_index];
}

void update_ppu(ppu_t *ppu) {
    ppu->cycles++;
    if (ppu->scanline < PPU_SCANLINE_IDLE) {
        render_ppu(ppu);
    } else if (ppu->scanline == PPU_SCANLINE_VBLANK && ppu->dot == 1) {
        // Enable VBlank
        ppu->status |= PPU_STATUS_VBLANK;
    } else if (ppu->scanline == PPU_SCANLINE_PRERENDER) {
        // Disable VBlank
        if (ppu->dot == 1) {
            ppu->status &= ~PPU_STATUS_VBLANK;
        }
        render_ppu(ppu);
    }

    // Check flags to enable NMI interrupt
    if ((ppu->status & PPU_STATUS_VBLANK) && (ppu->ctrl & PPU_CTRL_NMI)) {
        ppu->interrupt->nmi = true;
    }

    // Update counters
    ppu->dot++;
    bool skip_cycle = ppu->odd_frame && ppu->scanline == PPU_SCANLINE_PRERENDER;
    if ((ppu->dot == PPU_LINEDOTS - 1 && skip_cycle) ||
        (ppu->dot == PPU_LINEDOTS)) {
        ppu->dot = 0;
        ppu->scanline++;
    }
    if (ppu->scanline == PPU_SCANLINES) {
        ppu->scanline = 0;
        ppu->odd_frame = !ppu->odd_frame;
    }
}