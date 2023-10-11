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

    ppu->v = 0;
    ppu->t = 0;
    ppu->w = false;

    ppu->cycles = 21; // Initial cycle count
    ppu->scanline = 0;
    ppu->dot = ppu->cycles;

    ppu->odd_frame = false;

    ppu->bus = bus;
    ppu->interrupt = interrupt;
    ppu->suppress_vbl = false;
    ppu->suppress_nmi = false;

    create_event_tables_ppu(ppu);
}

void destroy_ppu(ppu_t *ppu) {}

void create_event_tables_ppu(ppu_t *ppu) {
    unsigned i;
    for (unsigned dot = 0; dot < PPU_LINEDOTS; dot++) {
        // Initialize all events to idle
        for (i = 0; i < PPU_EVENTS_PER_DOT; i++) {
            ppu->render_events[dot][i] = PPU_EVENT_IDLE;
            ppu->prerender_events[dot][i] = PPU_EVENT_IDLE;
            ppu->vblank_events[dot][i] = PPU_EVENT_IDLE;
            ppu->visible_events[dot][i] = PPU_EVENT_IDLE;
        }

        // Skip the first cycle of every scanline
        if (dot == 0) continue;

        // Prerender scanline events
        i = 0;
        if (dot == 1) {
            ppu->prerender_events[dot][i++] = PPU_EVENT_CLEAR_FLAGS;
        }
        if (dot >= 280 && dot <= 304) {
            ppu->prerender_events[dot][i++] = PPU_EVENT_COPY_Y;
        }
        if (dot == 338) {
            ppu->prerender_events[dot][i++] = PPU_EVENT_SKIP_CYCLE;
        }

        // VBlank scanline events
        i = 0;
        if (dot == 1) {
            ppu->vblank_events[dot][i++] = PPU_EVENT_SET_VBLANK;
        }

        // Render scanline events
        i = 0;
        if (dot <= 257 || dot >= 321) {
            switch (dot % 8) {
            case 1:
                if ((dot >= 9 && dot <= 257) || dot >= 329) {
                    ppu->render_events[dot][i++] = PPU_EVENT_RELOAD_SHIFTERS;
                }
                break;
            case 2:
                ppu->render_events[dot][i++] = PPU_EVENT_FETCH_NAME;
                break;
            case 4:
                ppu->render_events[dot][i++] = PPU_EVENT_FETCH_ATTRIBUTE;
                break;
            case 6:
                ppu->render_events[dot][i++] = PPU_EVENT_FETCH_PATTERN_LO;
                break;
            case 0:
                ppu->render_events[dot][i++] = PPU_EVENT_FETCH_PATTERN_HI;
                break;
            }
        }
        if ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337)) {
            ppu->render_events[dot][i++] = PPU_EVENT_SHIFT_REGISTERS;
        }
        if ((dot <= 256 || dot >= 328) && dot % 8 == 0) {
            ppu->render_events[dot][i++] = PPU_EVENT_INCREMENT_X;
        }
        if (dot == 256) {
            ppu->render_events[dot][i++] = PPU_EVENT_INCREMENT_Y;
        }
        if (dot == 257) {
            ppu->render_events[dot][i++] = PPU_EVENT_COPY_X;
        }
        if (dot >= 257 && dot <= 320) {
            ppu->render_events[dot][i++] = PPU_EVENT_CLEAR_OAMADDR;
        }
    }
}

unsigned char read_palette_ppu(ppu_t *ppu, unsigned char palette_index) {
    unsigned char value = ppu->palette[palette_index];
    if (ppu->mask & PPU_MASK_GREYSCALE) {
        value &= 0x30;
    }
    return value;
}

void write_palette_ppu(ppu_t *ppu,
                       unsigned char palette_index,
                       unsigned char value) {
    ppu->palette[palette_index] = value;
    if ((palette_index & 0x3) == 0) {
        ppu->palette[palette_index ^ 0x10] = value;
    }
}

void increment_vram_address_ppu(ppu_t *ppu) {
    ppu->v++;
    ppu->v += ((ppu->ctrl >> 2) & 1) * 31;
}

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

void shift_registers_ppu(ppu_t *ppu) {
    ppu->pt_shift[0] <<= 1;
    ppu->pt_shift[1] <<= 1;
    ppu->pa_shift[0] <<= 1;
    ppu->pa_shift[1] <<= 1;
}

void reload_shifters_ppu(ppu_t *ppu) {
    // Write to pattern table shift registers
    for (unsigned i = 0; i < 2; i++) {
        unsigned short pt_old = ppu->pt_shift[i] & 0xFF00;
        unsigned short pt_new = ppu->pt_latches[i];
        ppu->pt_shift[i] = pt_old | pt_new;
    }

    // Write to the palette shift registers
    for (unsigned i = 0; i < 2; i++) {
        bool bit = ppu->pa_latch & (i + 1);
        unsigned short pa_old = ppu->pa_shift[i] & 0xFF00;
        unsigned short pa_new = bit * 0x00FF;
        ppu->pa_shift[i] = pa_old | pa_new;
    }
}

void fetch_name_ppu(ppu_t *ppu) {
    address_t nt_address = 0x2000 | (ppu->v & 0x0FFF);
    ppu->nt_latch = read_ppu_bus(ppu->bus, nt_address);
}

void fetch_attribute_ppu(ppu_t *ppu) {
    address_t at_address = 0x23C0 | (ppu->v & 0x0C00) | ((ppu->v >> 4) & 0x38) |
                           ((ppu->v >> 2) & 0x07);
    unsigned char at = read_ppu_bus(ppu->bus, at_address);
    bool scroll_x = ppu->v & 0x02;
    bool scroll_y = ppu->v & 0x40;
    unsigned char quadrant = (scroll_y << 1) | scroll_x;
    ppu->pa_latch = (at >> (quadrant << 1)) & 0x03;
}

void fetch_pattern_lo_ppu(ppu_t *ppu) {
    bool bg_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_BG;
    address_t pt_base = bg_ctrl * 0x1000;
    address_t fine_y = (ppu->v >> 12) & 0x7;
    address_t pt_offset = (ppu->nt_latch << 4) | fine_y;
    address_t pt_address = pt_base + pt_offset;
    ppu->pt_latches[0] = read_ppu_bus(ppu->bus, pt_address);
}

void fetch_pattern_hi_ppu(ppu_t *ppu) {
    bool bg_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_BG;
    address_t pt_base = bg_ctrl * 0x1000;
    address_t fine_y = (ppu->v >> 12) & 0x7;
    address_t pt_offset = (ppu->nt_latch << 4) | fine_y;
    address_t pt_address = pt_base + pt_offset + 8;
    ppu->pt_latches[1] = read_ppu_bus(ppu->bus, pt_address);
}

void increment_x_ppu(ppu_t *ppu) {
    if ((ppu->v & 0x001F) == 31) {
        ppu->v &= ~0x001F;
        ppu->v ^= 0x0400;
    } else {
        ppu->v += 1;
    }
}

void increment_y_ppu(ppu_t *ppu) {
    if ((ppu->v & 0x7000) != 0x7000) {
        ppu->v += 0x1000;
    } else {
        ppu->v &= ~0x7000;
        unsigned y = (ppu->v & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            ppu->v ^= 0x0800;
        } else if (y == 31) {
            y = 0;
        } else {
            y += 1;
        }
        ppu->v = (ppu->v & ~0x03E0) | (y << 5);
    }
}

void copy_x_ppu(ppu_t *ppu) {
    unsigned short mask = 0x041F;
    ppu->v &= ~mask;
    ppu->v |= (ppu->t & mask);
}

void copy_y_ppu(ppu_t *ppu) {
    unsigned short mask = 0x7BE0;
    ppu->v &= ~mask;
    ppu->v |= (ppu->t & mask);
}

void set_vblank_ppu(ppu_t *ppu) {
    if (!ppu->suppress_vbl) {
        ppu->status |= PPU_STATUS_VBLANK;

        // Trigger NMI if enabled.
        if ((ppu->ctrl & PPU_CTRL_NMI) && !ppu->suppress_nmi) {
            set_nmi_interrupt(ppu->interrupt, true);
        }
    }
}

void draw_dot_ppu(ppu_t *ppu) {
    unsigned short x_mask = 0x8000 >> ppu->x;

    // Read palette number from attribute table
    bool pa0 = ppu->pa_shift[0] & x_mask;
    bool pa1 = ppu->pa_shift[1] & x_mask;
    unsigned char palette = pa0 | (pa1 << 1);

    // Read color number from pattern table
    bool pt0 = ppu->pt_shift[0] & x_mask;
    bool pt1 = ppu->pt_shift[1] & x_mask;
    unsigned char color = pt0 | (pt1 << 1);

    // Fetch the actual color value from the palette
    unsigned char palette_index = (palette << 2) | color;
    unsigned char palette_byte = read_palette_ppu(ppu, palette_index);

    // Write to the color buffer
    unsigned buffer_index = ppu->scanline * PPU_LINEDOTS + ppu->dot;
    ppu->color_buffer[buffer_index] =
        create_color(palette_byte,
                     ppu->mask & PPU_MASK_GREYSCALE,
                     ppu->mask & PPU_MASK_EMPHASIZE_RED,
                     ppu->mask & PPU_MASK_EMPHASIZE_GREEN,
                     ppu->mask & PPU_MASK_EMPHASIZE_BLUE);
}

void execute_events_ppu(ppu_t *ppu, ppu_event_t *events) {
    bool enabled = ppu->mask & (PPU_MASK_SHOW_BG | PPU_MASK_SHOW_SPRITES);
    unsigned i = 0;
    while (i < PPU_EVENTS_PER_DOT && events[i] != PPU_EVENT_IDLE) {
        switch (events[i]) {
        case PPU_EVENT_SHIFT_REGISTERS:
            if (enabled) {
                shift_registers_ppu(ppu);
            }
            break;
        case PPU_EVENT_RELOAD_SHIFTERS:
            if (enabled) {
                reload_shifters_ppu(ppu);
            }
            break;
        case PPU_EVENT_FETCH_NAME:
            if (enabled) {
                fetch_name_ppu(ppu);
            }
            break;
        case PPU_EVENT_FETCH_ATTRIBUTE:
            if (enabled) {
                fetch_attribute_ppu(ppu);
            }
            break;
        case PPU_EVENT_FETCH_PATTERN_LO:
            if (enabled) {
                fetch_pattern_lo_ppu(ppu);
            }
            break;
        case PPU_EVENT_FETCH_PATTERN_HI:
            if (enabled) {
                fetch_pattern_hi_ppu(ppu);
            }
            break;
        case PPU_EVENT_INCREMENT_X:
            if (enabled) {
                increment_x_ppu(ppu);
            }
            break;
        case PPU_EVENT_INCREMENT_Y:
            if (enabled) {
                increment_y_ppu(ppu);
            }
            break;
        case PPU_EVENT_COPY_X:
            if (enabled) {
                copy_x_ppu(ppu);
            }
            break;
        case PPU_EVENT_COPY_Y:
            if (enabled) {
                copy_y_ppu(ppu);
            }
            break;
        case PPU_EVENT_CLEAR_OAMADDR:
            if (enabled) {
                ppu->oam_addr = 0;
            }
            break;
        case PPU_EVENT_CLEAR_FLAGS:
            ppu->status &= ~(PPU_STATUS_VBLANK | PPU_STATUS_S_OVERFLOW |
                             PPU_STATUS_S0_HIT);
            break;
        case PPU_EVENT_SKIP_CYCLE:
            ppu->dot += ppu->odd_frame && enabled;
            break;
        case PPU_EVENT_SET_VBLANK:
            set_vblank_ppu(ppu);
            break;
        case PPU_EVENT_IDLE:
            break;
        }
        i++;
    }
}

void advance_frame_ppu(ppu_t *ppu) {
    ppu->scanline = 0;
    ppu->odd_frame = !ppu->odd_frame;

    // Clear the IO latch every frame (to simulate value decay)
    ppu->io_databus = 0;
}

void update_ppu(ppu_t *ppu) {
    // Execute events
    switch (ppu->scanline) {
    case PPU_SCANLINES:
        advance_frame_ppu(ppu);
        break;
    case PPU_SCANLINE_PRERENDER:
        execute_events_ppu(ppu, ppu->prerender_events[ppu->dot]);
        execute_events_ppu(ppu, ppu->render_events[ppu->dot]);
        break;
    case PPU_SCANLINE_VBLANK:
        execute_events_ppu(ppu, ppu->vblank_events[ppu->dot]);
        break;
    default:
        if (ppu->scanline < PPU_SCANLINE_IDLE) {
            execute_events_ppu(ppu, ppu->render_events[ppu->dot]);
        }
        break;
    }
    draw_dot_ppu(ppu);

    // Handle suppressing NMI
    if (ppu->suppress_nmi) {
        set_nmi_interrupt(ppu->interrupt, false);
    }

    // Reset suppression flags
    ppu->suppress_vbl = false;
    ppu->suppress_nmi = false;

    // Update counters and advance the scanline
    ppu->cycles++;
    ppu->dot++;
    if (ppu->dot == PPU_LINEDOTS) {
        ppu->dot = 0;
        ppu->scanline++;
    }
}