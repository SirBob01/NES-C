#include "./ppu.h"

void create_ppu(ppu_t *ppu, ppu_bus_t *bus, interrupt_t *interrupt) {
    ppu->ctrl = 0;
    ppu->mask = 0;
    ppu->status = PPU_STATUS_VBLANK | PPU_STATUS_S_OVERFLOW;
    ppu->oamaddr = 0;

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

    ppu->sprite_index = 0;
    ppu->sprite_m = 0;
    ppu->sprite_count = 0;
    ppu->sprite_count_latch = 0;

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

        // Sprite events
        if (dot <= 64 && dot % 2 == 1) {
            ppu->render_events[dot][i++] = PPU_EVENT_CLEAR_OAM;
        }
        if (dot >= 65 && dot <= 256) {
            ppu->render_events[dot][i++] = PPU_EVENT_EVALUATE_SPRITES;
        }
        if (dot >= 257 && dot <= 320) {
            switch (dot % 8) {
            case 4:
                ppu->render_events[dot][i++] = PPU_EVENT_FETCH_ATTRIBUTE_SPRITE;
                break;
            case 6:
                ppu->render_events[dot][i++] =
                    PPU_EVENT_FETCH_PATTERN_SPRITE_LO;
                break;
            case 0:
                ppu->render_events[dot][i++] =
                    PPU_EVENT_FETCH_PATTERN_SPRITE_HI;
                break;
            }
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

unsigned char read_primary_oam_ppu(ppu_t *ppu, unsigned char oam_index) {
    if (ppu->dot <= 64 && ppu->dot >= 1 && is_rendering_ppu(ppu)) {
        return 0xFF;
    } else {
        return ppu->primary_oam[oam_index];
    }
}

unsigned char read_status_ppu(ppu_t *ppu) {
    // Set low 5 bits of status register to io latch
    unsigned char result = ppu->status;
    result &= ~0x1F;
    result |= ppu->io_databus & 0x1F;
    ppu->io_databus = result;

    // Update registers
    ppu->status &= ~PPU_STATUS_VBLANK;
    ppu->w = false;

    // Suppress setting VBlank and NMI if reading too close
    if (ppu->scanline == PPU_SCANLINE_VBLANK) {
        if (ppu->dot <= 1) {
            ppu->suppress_vbl = true;
        }
        if (ppu->dot <= 3) {
            ppu->suppress_nmi = true;
        }
    }
    return result;
}

unsigned char read_oamdata_ppu(ppu_t *ppu) {
    unsigned char result = read_primary_oam_ppu(ppu, ppu->oamaddr);
    if ((ppu->oamaddr & 3) == 2) {
        result &= 0xE3;
    }
    ppu->io_databus = result;
    return result;
}

unsigned char read_data_ppu(ppu_t *ppu) {
    address_t vram_addr = ppu->v & 0x3FFF;
    unsigned char result = ppu->buffer2007;
    ppu->buffer2007 = read_ppu_bus(ppu->bus, vram_addr);

    // Set high 2 bits from palette to io latch and apply PPU effects
    if (ppu->v >= PPU_MAP_PALETTE && !is_rendering_ppu(ppu)) {
        address_t palette_addr = ppu->v & 0x1F;
        result = read_palette_ppu(ppu, palette_addr);
        result &= ~0xC0;
        result |= ppu->io_databus & 0xC0;
    }
    ppu->v++;
    ppu->v += ((ppu->ctrl >> 2) & 1) * 31;
    ppu->io_databus = result;
    return result;
}

void write_ctrl_ppu(ppu_t *ppu, unsigned char value) {
    unsigned char prev_value = ppu->ctrl;
    ppu->ctrl = value;

    address_t gh = (value & 0x03) << 10;
    ppu->t = (ppu->t & 0x73FF) | gh;
    ppu->io_databus = value;

    // Disable NMI if cleared near VBlank disable
    if ((prev_value & PPU_CTRL_NMI) && !(ppu->ctrl & PPU_CTRL_NMI) &&
        ppu->dot <= 3) {
        set_nmi_interrupt(ppu->interrupt, false);
    }

    // Trigger NMI if VBlank is set
    if ((ppu->status & PPU_STATUS_VBLANK) && (ppu->ctrl & PPU_CTRL_NMI) &&
        !(prev_value & PPU_CTRL_NMI) && ppu->dot != 1) {
        set_nmi_interrupt(ppu->interrupt, true);
    }
}

void write_mask_ppu(ppu_t *ppu, unsigned char value) {
    ppu->mask = value;
    ppu->io_databus = value;
}

void write_oamaddr_ppu(ppu_t *ppu, unsigned char value) {
    ppu->oamaddr = value;
    ppu->io_databus = value;
}

void write_oamdata_ppu(ppu_t *ppu, unsigned char value) {
    if (is_rendering_ppu(ppu)) {
        ppu->oamaddr += 0x04; // Bump high 6 bits
    } else {
        ppu->primary_oam[ppu->oamaddr++] = value;
    }
    ppu->io_databus = value;
}

void write_scroll_ppu(ppu_t *ppu, unsigned char value) {
    if (!ppu->w) {
        address_t abcde = (value & 0xF8) >> 3;
        address_t fgh = value & 0x07;

        ppu->t = (ppu->t & 0xFFE0) | abcde;
        ppu->x = fgh;
    } else {
        address_t ab = (value & 0xC0) << 2;
        address_t cde = (value & 0x38) << 2;
        address_t fgh = (value & 0x07) << 12;
        address_t tmask = ab | cde | fgh;

        ppu->t = (ppu->t & 0xC1F) | tmask;
    }
    ppu->w = !ppu->w;
    ppu->io_databus = value;
}

void write_addr_ppu(ppu_t *ppu, unsigned char value) {
    if (!ppu->w) {
        address_t hi = value & 0x3F;
        address_t lo = ppu->t & 0x00FF;
        ppu->t = (hi << 8) | lo;
    } else {
        ppu->t = (ppu->t & 0x7F00) | value;
        ppu->v = ppu->t;
    }
    ppu->w = !ppu->w;
    ppu->io_databus = value;
}

void write_data_ppu(ppu_t *ppu, unsigned char value) {
    if (ppu->v >= PPU_MAP_PALETTE && !is_rendering_ppu(ppu)) {
        address_t palette_addr = ppu->v & 0x1F;
        ppu->palette[palette_addr] = value & 0x3F; // Only include lower 6 bits
        if ((palette_addr & 0x3) == 0) {
            ppu->palette[palette_addr ^ 0x10] = ppu->palette[palette_addr];
        }
    } else {
        address_t vram_addr = ppu->v & 0x3FFF;
        write_ppu_bus(ppu->bus, vram_addr, value);
    }
    ppu->v++;
    ppu->v += ((ppu->ctrl >> 2) & 1) * 31;
    ppu->io_databus = value;

    // TODO:
    // https://www.nesdev.org/wiki/PPU_scrolling#$2007_reads_and_writes
}

void read_state_ppu(ppu_t *ppu, char *buffer, unsigned buffer_size) {
    snprintf(buffer,
             buffer_size,
             "CTRL:%02X STATUS:%02X MASK:%02X OAMADDR:%02X CYC:%3d,%3d",
             ppu->ctrl,
             ppu->status,
             ppu->mask,
             ppu->oamaddr,
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
    address_t pt_address = pt_base | (ppu->nt_latch << 4) | fine_y;
    ppu->pt_latches[0] = read_ppu_bus(ppu->bus, pt_address);
}

void fetch_pattern_hi_ppu(ppu_t *ppu) {
    bool bg_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_BG;
    address_t pt_base = bg_ctrl * 0x1000;
    address_t fine_y = (ppu->v >> 12) & 0x7;
    address_t pt_address = pt_base | (ppu->nt_latch << 4) | fine_y;
    ppu->pt_latches[1] = read_ppu_bus(ppu->bus, pt_address + 8);
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

void clear_oam_ppu(ppu_t *ppu) {
    ppu->secondary_oam[(ppu->dot - 1) >> 1] = 0xFF;
    ppu->sprite_count = 0;
    ppu->sprite_index = ppu->oamaddr >> 2;
    ppu->sprite_m = ppu->oamaddr & 3;
}

bool sprite_in_range_ppu(ppu_t *ppu, unsigned char y) {
    bool sprite_size = ppu->ctrl & PPU_CTRL_SPRITE_SIZE;
    unsigned height = (sprite_size + 1) * 8;
    unsigned max = y + height;
    return y < 255 && ppu->scanline >= y && ppu->scanline < max;
}

void evaluate_sprites_ppu(ppu_t *ppu) {
    // TODO: This algorithm is iffy
    if (ppu->sprite_index >= 64) {
        return;
    }
    unsigned pri_index = ppu->sprite_index * 4 + ppu->sprite_m;
    unsigned sec_index = ppu->sprite_count * 4 + ppu->sprite_m;

    if (ppu->dot % 2) {
        ppu->buffer_oam = read_primary_oam_ppu(ppu, pri_index);
    } else {
        if (ppu->sprite_count < 8) {
            ppu->secondary_oam[sec_index] = ppu->buffer_oam;
        }
        if (ppu->sprite_m == 0) {
            if (sprite_in_range_ppu(ppu, ppu->buffer_oam)) {
                if (ppu->sprite_count == 8) {
                    ppu->status |= PPU_STATUS_S_OVERFLOW;
                }
                ppu->sprite_m++;
            } else {
                ppu->sprite_index++;
            }
        } else {
            ppu->sprite_m++;
            if (ppu->sprite_m == 4 && ppu->sprite_count < 8) {
                ppu->sprite_indices[ppu->sprite_count] = ppu->sprite_index;
                ppu->sprite_m = 0;
                ppu->sprite_index++;
                ppu->sprite_count++;
            }
        }
    }
}

void fetch_sprite_attribute_ppu(ppu_t *ppu) {
    unsigned sprite_index = (ppu->dot - 256) >> 3;
    unsigned char attr = ppu->secondary_oam[sprite_index * 4 + 2];
    unsigned char x = ppu->secondary_oam[sprite_index * 4 + 3];
    ppu->sprite_latches[sprite_index] = attr;
    ppu->sprite_counters[sprite_index] = x;
    ppu->sprite_count_latch = ppu->sprite_count;
}

void fetch_sprite_pattern_lo_ppu(ppu_t *ppu) {
    unsigned sprite_index = (ppu->dot - 256) >> 3;
    unsigned char y = ppu->scanline - ppu->secondary_oam[sprite_index * 4];
    unsigned char tile = ppu->secondary_oam[sprite_index * 4 + 1];
    unsigned char attr = ppu->sprite_latches[sprite_index];

    bool flip_x = attr & 0x40;
    bool flip_y = attr & 0x80;
    bool sprite_size = ppu->ctrl & PPU_CTRL_SPRITE_SIZE;
    bool sprite_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_SPRITE;
    address_t pt_base = 0x1000;
    if (sprite_size) {
        pt_base *= tile & 1;
        tile >>= 1;
        if (flip_y) {
            y = 15 - y;
        }
    } else {
        pt_base *= sprite_ctrl;
        if (flip_y) {
            y = 7 - y;
        }
    }
    address_t pt_address = pt_base | (tile << 4) | y;
    unsigned char byte = read_ppu_bus(ppu->bus, pt_address);
    if (flip_x) {
        byte = reverse_bits(byte);
    }
    ppu->sprite_shift[sprite_index * 2] = byte;
}

void fetch_sprite_pattern_hi_ppu(ppu_t *ppu) {
    unsigned sprite_index = (ppu->dot - 258) >> 3;
    unsigned char y = ppu->scanline - ppu->secondary_oam[sprite_index * 4];
    unsigned char tile = ppu->secondary_oam[sprite_index * 4 + 1];
    unsigned char attr = ppu->sprite_latches[sprite_index];

    bool flip_x = attr & 0x40;
    bool flip_y = attr & 0x80;
    bool sprite_size = ppu->ctrl & PPU_CTRL_SPRITE_SIZE;
    bool sprite_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_SPRITE;
    address_t pt_base = 0x1000;
    if (sprite_size) {
        pt_base *= tile & 1;
        tile >>= 1;
        if (flip_y) {
            y = 15 - y;
        }
    } else {
        pt_base *= sprite_ctrl;
        if (flip_y) {
            y = 7 - y;
        }
    }
    address_t pt_address = pt_base | (tile << 4) | y;
    unsigned char byte = read_ppu_bus(ppu->bus, pt_address + 8);
    if (flip_x) {
        byte = reverse_bits(byte);
    }
    ppu->sprite_shift[sprite_index * 2 + 1] = byte;
}

void draw_dot_ppu(ppu_t *ppu) {
    unsigned short x_mask = 0x8000 >> ppu->x;

    bool bg_pa0 = ppu->pa_shift[0] & x_mask;
    bool bg_pa1 = ppu->pa_shift[1] & x_mask;

    bool bg_pt0 = ppu->pt_shift[0] & x_mask;
    bool bg_pt1 = ppu->pt_shift[1] & x_mask;

    // Get background color (and mask if necessary)
    unsigned char bg_palette = bg_pa0 | (bg_pa1 << 1);
    unsigned char bg_color = bg_pt0 | (bg_pt1 << 1);
    if ((ppu->dot <= 7 && !(ppu->mask & PPU_MASK_SHOW_BG_LEFT)) ||
        !(ppu->mask & PPU_MASK_SHOW_BG)) {
        bg_color = 0;
    }

    // Process each sprite in order of priority
    unsigned char sp_palette = 0;
    unsigned char sp_color = 0;
    bool sp_behind_bg = true;

    if ((ppu->mask & PPU_MASK_SHOW_SPRITES) &&
        (ppu->dot > 7 || (ppu->mask & PPU_MASK_SHOW_SPRITES_LEFT)) &&
        ppu->dot < 255) {
        unsigned char min_sp_index = 0xFF;
        for (int i = 0; i < ppu->sprite_count_latch; i++) {
            if (ppu->sprite_counters[i] == 0) {
                // Shift active sprite registers
                ppu->sprite_shift[i * 2] <<= 1;
                ppu->sprite_shift[i * 2 + 1] <<= 1;

                unsigned char sp_index = ppu->sprite_indices[i];
                unsigned char sp_attr = ppu->sprite_latches[i];

                // Get sprite color (and mask if necessary)
                bool sp_pt0 = ppu->sprite_shift[i * 2] & 0x100;
                bool sp_pt1 = ppu->sprite_shift[i * 2 + 1] & 0x100;
                unsigned char sp_color_tmp = sp_pt0 | (sp_pt1 << 1);

                // Detect sprite 0 hit
                if (sp_index == 0 && bg_color && sp_color_tmp) {
                    ppu->status |= PPU_STATUS_S0_HIT;
                }

                // Update palette index depending on pixel priority
                if (sp_index <= min_sp_index && sp_color_tmp) {
                    min_sp_index = sp_index;
                    sp_palette = (sp_attr & 0x3) + 4;
                    sp_color = sp_color_tmp;
                    sp_behind_bg = sp_attr & 0x20;
                }
            } else {
                ppu->sprite_counters[i]--;
            }
        }
    }

    // Multiplexer
    unsigned char palette_index = (bg_palette << 2) | bg_color;
    if (!bg_color || (sp_color && !sp_behind_bg)) {
        palette_index = (sp_palette << 2) | sp_color;
    }

    // Write to the color buffer
    unsigned char palette_value = read_palette_ppu(ppu, palette_index);
    unsigned buffer_index = ppu->scanline * PPU_LINEDOTS + ppu->dot;
    ppu->color_buffer[buffer_index] =
        create_color(palette_value,
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
        case PPU_EVENT_CLEAR_OAM:
            if (enabled) {
                clear_oam_ppu(ppu);
            }
            break;
        case PPU_EVENT_EVALUATE_SPRITES:
            if (enabled) {
                evaluate_sprites_ppu(ppu);
            }
            break;
        case PPU_EVENT_FETCH_ATTRIBUTE_SPRITE:
            if (enabled) {
                fetch_sprite_attribute_ppu(ppu);
            }
            break;
        case PPU_EVENT_FETCH_PATTERN_SPRITE_LO:
            if (enabled) {
                fetch_sprite_pattern_lo_ppu(ppu);
            }
            break;
        case PPU_EVENT_FETCH_PATTERN_SPRITE_HI:
            if (enabled) {
                fetch_sprite_pattern_hi_ppu(ppu);
            }
            break;
        case PPU_EVENT_CLEAR_OAMADDR:
            if (enabled) {
                ppu->oamaddr = 0;
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