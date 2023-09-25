#include "./ppu.h"

void create_ppu(ppu_t *ppu, ppu_bus_t *bus, interrupt_t *interrupt) {
    ppu->ctrl = 0;
    ppu->mask = 0;
    ppu->status = PPU_STATUS_VBLANK | PPU_STATUS_S_OVERFLOW;
    ppu->oam_addr = 0;
    ppu->oam_data = 0;
    ppu->scroll = 0;
    ppu->addr = 0;
    ppu->data = 0;
    ppu->oam_dma = 0;

    ppu->internal.v = 0;
    ppu->internal.t = 0;
    ppu->internal.w = false;

    ppu->cycles = 21; // Initial cycle count
    ppu->scanline = 0;
    ppu->dot = ppu->cycles;

    ppu->odd_frame = false;

    ppu->bus = bus;
    ppu->interrupt = interrupt;
    ppu->suppress_vbl = false;
    ppu->suppress_nmi = false;
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

bool is_rendering_ppu(ppu_t *ppu) {
    bool flags = ppu->mask & (PPU_MASK_SHOW_BG | PPU_MASK_SHOW_SPRITES);
    return flags && (ppu->scanline < PPU_SCANLINE_IDLE ||
                     ppu->scanline == PPU_SCANLINE_PRERENDER);
}

void render_ppu(ppu_t *ppu) {
    // TODO: Compute this
    unsigned palette_index = rand() % 64;

    // Write to the color buffer
    unsigned buffer_index = ppu->scanline * PPU_LINEDOTS + ppu->dot;
    ppu->color_buffer[buffer_index] = COLOR_PALETTE[palette_index];
}

void update_ppu(ppu_t *ppu) {
    bool is_rendering = is_rendering_ppu(ppu);
    if (is_rendering) {
        render_ppu(ppu);

        // Clear OAMADDR
        if (ppu->dot >= 257 && ppu->dot <= 320) {
            ppu->oam_addr = 0;
        }
    }

    // Update status flags
    switch (ppu->scanline) {
    case PPU_SCANLINE_PRERENDER:
        if (ppu->dot == 1) {
            ppu->status &= ~(PPU_STATUS_VBLANK | PPU_STATUS_S_OVERFLOW |
                             PPU_STATUS_S0_HIT);
        }
        break;
    case PPU_SCANLINE_VBLANK:
        if (ppu->dot == 1 && !ppu->suppress_vbl) {
            ppu->status |= PPU_STATUS_VBLANK;

            // Trigger NMI if enabled.
            if ((ppu->ctrl & PPU_CTRL_NMI) && !ppu->suppress_nmi) {
                set_nmi_interrupt(ppu->interrupt, true);
            }
        }
        break;
    }

    // Handle suppressing NMI
    if (ppu->suppress_nmi) {
        set_nmi_interrupt(ppu->interrupt, false);
    }

    // Reset suppression flags
    ppu->suppress_vbl = false;
    ppu->suppress_nmi = false;

    // Update counters
    ppu->cycles++;
    ppu->dot++;
    bool skip_cycle = ppu->odd_frame &&
                      ppu->scanline == PPU_SCANLINE_PRERENDER && is_rendering;
    if ((ppu->dot == PPU_LINEDOTS - 1 && skip_cycle) ||
        (ppu->dot == PPU_LINEDOTS)) {
        ppu->dot = 0;
        ppu->scanline++;
    }
    if (ppu->scanline == PPU_SCANLINES) {
        ppu->scanline = 0;
        ppu->odd_frame = !ppu->odd_frame;

        // Clear the IO latch every frame (to simulate value decay)
        ppu->io_databus = 0;
    }
}