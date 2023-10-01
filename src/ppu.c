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

void fetch_nametable(ppu_t *ppu) {
    address_t nt_base = 0x2000 + 0x400 * (ppu->ctrl & PPU_CTRL_NAMETABLE_BASE);
    address_t nt_offset = ppu->internal.v & 0x0FFF;
    address_t nt_address = nt_base + nt_offset;
    ppu->internal.nt_latch = read_ppu_bus(ppu->bus, nt_address);
}

void fetch_attribute_table(ppu_t *ppu) {
    address_t nt_base = 0x2000 + 0x400 * (ppu->ctrl & PPU_CTRL_NAMETABLE_BASE);
    address_t at_base = nt_base + 0x3C0;
    address_t at_address = at_base | (ppu->internal.v & 0x0C00) |
                           ((ppu->internal.v >> 4) & 0x38) |
                           ((ppu->internal.v >> 2) & 0x07);
    ppu->internal.pa_latch = read_ppu_bus(ppu->bus, at_address);
}

void fetch_pattern_table_low(ppu_t *ppu) {
    bool bg_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_BG;
    address_t pt_base = bg_ctrl * 0x1000;
    address_t tile_row = (ppu->internal.v & 0x7000) >> 12;
    address_t pt_offset = ppu->internal.nt_latch * 16 + tile_row;
    address_t pt_address = pt_base + pt_offset;

    ppu->internal.pt_latches[0] = read_ppu_bus(ppu->bus, pt_address);
}

void fetch_pattern_table_high(ppu_t *ppu) {
    bool bg_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_BG;
    address_t pt_base = bg_ctrl * 0x1000;
    address_t tile_row = (ppu->internal.v & 0x7000) >> 12;
    address_t pt_offset = ppu->internal.nt_latch * 16 + tile_row;
    address_t pt_address = pt_base + pt_offset;

    ppu->internal.pt_latches[1] = read_ppu_bus(ppu->bus, pt_address + 8);
}

void increment_x(ppu_t *ppu) {
    if ((ppu->internal.v & 0x001F) == 31) {
        ppu->internal.v &= ~0x001F;
        ppu->internal.v ^= 0x0400;
    } else {
        ppu->internal.v += 1;
    }
}

void increment_y(ppu_t *ppu) {
    if ((ppu->internal.v & 0x7000) != 0x7000) {
        ppu->internal.v += 0x1000;
    } else {
        ppu->internal.v &= ~0x7000;
        int y = (ppu->internal.v & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            ppu->internal.v ^= 0x0800;
        } else if (y == 31) {
            y = 0;
        } else {
            y += 1;
            ppu->internal.v = (ppu->internal.v & ~0x03E0) | (y << 5);
        }
    }
}

void shift_registers(ppu_t *ppu) {
    // Write to pattern table shift registers
    for (unsigned i = 0; i < 2; i++) {
        unsigned short pt_old = ppu->internal.pt_shift[i] & 0xFF00;
        unsigned short pt_new = ppu->internal.pt_latches[i];
        ppu->internal.pt_shift[i] = pt_old | pt_new;
    }
}

void render_ppu(ppu_t *ppu) {
    // Perform memory accesses every other dot
    switch (ppu->dot % 8) {
    case 0:
        shift_registers(ppu);
        break;
    case 1:
        fetch_nametable(ppu);
        break;
    case 3:
        fetch_attribute_table(ppu);
        break;
    case 5:
        fetch_pattern_table_low(ppu);
        break;
    case 7:
        fetch_pattern_table_high(ppu);
        break;
    }

    // Read palette number from attribute table
    unsigned char palette = ppu->internal.pa_latch & 0x3;

    // Read color number from pattern table
    unsigned short x_mask = 0x8000 >> ppu->internal.x;
    bool pt0 = ppu->internal.pt_shift[0] & x_mask;
    bool pt1 = ppu->internal.pt_shift[1] & x_mask;
    unsigned char color = pt0 | (pt1 << 1);

    // Fetch the actual color value from the palette
    address_t color_address = PPU_MAP_PALETTE | (palette << 2) | color;
    unsigned char color_index = read_ppu_bus(ppu->bus, color_address);

    // Write to the color buffer
    unsigned buffer_index = ppu->scanline * PPU_LINEDOTS + ppu->dot;
    ppu->color_buffer[buffer_index] = COLOR_PALETTE[color_index];

    // Shift registers
    ppu->internal.pt_shift[0] <<= 1;
    ppu->internal.pt_shift[1] <<= 1;
}

void update_ppu(ppu_t *ppu) {
    bool is_rendering = is_rendering_ppu(ppu);
    if (is_rendering) {
        // Render the current pixel to the color buffer
        render_ppu(ppu);

        // Clear OAMADDR
        if (ppu->dot >= 257 && ppu->dot <= 320) {
            ppu->oam_addr = 0;
        }

        // Increment y
        if (ppu->dot == 256) {
            increment_y(ppu);
        }

        // Copy horizontal bits from t to v
        if (ppu->dot == 257) {
            unsigned short mask = 0x041F;
            ppu->internal.v &= ~mask;
            ppu->internal.v |= (ppu->internal.t & mask);
        }

        // Copy vertical bits from t to v
        if (ppu->dot >= 280 && ppu->dot <= 304 &&
            ppu->scanline == PPU_SCANLINE_PRERENDER) {
            unsigned short mask = 0x7BE0;
            ppu->internal.v &= ~mask;
            ppu->internal.v |= (ppu->internal.t & mask);
        }

        // Increment x
        if (ppu->dot >= 328 || ppu->dot <= 256) {
            if (ppu->dot % 8 == 0) increment_x(ppu);
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

    // Handle skip cycle
    if (is_rendering && ppu->odd_frame &&
        ppu->scanline == PPU_SCANLINE_PRERENDER && ppu->dot == 339) {
        ppu->dot++;
    }

    // New scanline
    if (ppu->dot == PPU_LINEDOTS) {
        ppu->dot = 0;
        ppu->scanline++;
    }

    // New frame
    if (ppu->scanline == PPU_SCANLINES) {
        ppu->scanline = 0;
        ppu->odd_frame = !ppu->odd_frame;

        // Clear the IO latch every frame (to simulate value decay)
        ppu->io_databus = 0;
    }
}