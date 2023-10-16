#include "./io.h"

void create_io(io_t *io, emulator_t *emu) {
    io->emu = emu;
    create_display(&io->display, 256, 240, "NES-C");
    create_audio(&io->audio, &emu->apu.buffer);
    create_input(&io->input);
}

void destroy_io(io_t *io) {
    if (is_debug_io(io)) {
        destroy_display(&io->pattern_table);
        destroy_display(&io->nametables);
    }
    destroy_display(&io->display);
    destroy_input(&io->input);
    destroy_audio(&io->audio);
}

bool is_debug_io(io_t *io) {
    // Check if debug displays are being rendered
    return !is_free_memory(&io->pattern_table.bitmap) &&
           !is_free_memory(&io->nametables.bitmap);
}

void set_debug_io(io_t *io, bool debug) {
    if (is_debug_io(io) == debug) return;
    if (debug) {
        create_display(&io->pattern_table, 128, 256, "Pattern Tables");
        create_display(&io->nametables, 32 * 2 * 8, 30 * 2 * 8, "Nametables");
    } else {
        destroy_display(&io->pattern_table);
        destroy_display(&io->nametables);
    }
}

void debug_io(io_t *io, emulator_t *emu) {
    // Draw tile grid
    const color_t grid_color = {0xff, 0, 0};
    for (unsigned x = 0; x < io->display.size.x; x++) {
        for (unsigned y = 0; y < io->display.size.y; y++) {
            if (x % 8 == 0 || y % 8 == 0) {
                vec2_t position = {x, y};
                draw_display(&io->display, position, grid_color);
            }
        }
    }

    // Draw pattern tables
    unsigned char *chr_rom = get_chr_rom(&emu->rom);
    // For each tile
    for (unsigned i = 0; i < 512; i++) {
        // For each row
        for (unsigned y = 0; y < 8; y++) {
            // For each column
            unsigned char plane0 = chr_rom[i * 16 + y];
            unsigned char plane1 = chr_rom[i * 16 + y + 8];

            for (unsigned x = 0; x < 8; x++) {
                unsigned char pixel = ((plane0 >> (7 - x)) & 1) |
                                      (((plane1 >> (7 - x)) & 1) << 1);
                vec2_t position = {
                    (i & 15) * 8 + x,
                    (i >> 4) * 8 + y,
                };
                color_t color = {0, 0, 0};
                switch (pixel) {
                case 1:
                    color.r = 0xff;
                    break;
                case 2:
                    color.g = 0xff;
                    break;
                case 3:
                    color.b = 0xff;
                    break;
                default:
                    break;
                }
                // Draw the pixel
                draw_display(&io->pattern_table, position, color);
            }
        }
    }
    refresh_display(&io->pattern_table);

    // Draw nametables
    address_t bases[4] = {
        0x2000,
        0x2400,
        0x2800,
        0x2C00,
    };
    ppu_t *ppu = &io->emu->ppu;
    for (unsigned b = 0; b < 0x3C0; b++) {
        for (unsigned n = 0; n < 4; n++) {
            address_t address = bases[n] + b;
            unsigned char tile = read_ppu_bus(&emu->ppu_bus, address);

            unsigned x_tile_offset = (b & 31) + (n & 1) * 32;
            unsigned y_tile_offset = (b >> 5) + (n >> 1) * 30;

            for (unsigned y = 0; y < 8; y++) {
                address_t at_address = 0x23C0 | (b & 0x0C00) |
                                       ((b >> 4) & 0x38) | ((b >> 2) & 0x07);
                unsigned char at = read_ppu_bus(ppu->bus, at_address);
                bool scroll_x = b & 0x02;
                bool scroll_y = b & 0x40;
                unsigned char quadrant = (scroll_y << 1) | scroll_x;
                unsigned char palette = (at >> (quadrant << 1)) & 0x03;

                bool bg_ctrl = ppu->ctrl & PPU_CTRL_PATTERN_TABLE_BG;
                address_t pt_address = (bg_ctrl * 0x1000) | (tile << 4) | y;
                unsigned char lo = read_ppu_bus(ppu->bus, pt_address);
                unsigned char hi = read_ppu_bus(ppu->bus, pt_address + 8);

                for (unsigned x = 0; x < 8; x++) {
                    unsigned char palette_index = (palette << 2) |
                                                  ((lo >> (7 - x)) & 1) |
                                                  (((hi >> (7 - x)) & 1) << 1);
                    vec2_t position = {
                        x_tile_offset * 8 + x,
                        y_tile_offset * 8 + y,
                    };
                    color_t color = create_color(ppu->palette[palette_index],
                                                 false,
                                                 false,
                                                 false,
                                                 false);
                    draw_display(&io->nametables, position, color);
                }
            }
        }
    }
    refresh_display(&io->nametables);
}

bool refresh_io(io_t *io, emulator_t *emu) {
    poll_input(&io->input);

    // Draw the color buffer from the PPU.
    for (unsigned x = 0; x < io->display.size.x; x++) {
        for (unsigned y = 0; y < io->display.size.y; y++) {
            unsigned i = x + y * PPU_LINEDOTS;
            vec2_t position = {x, y};
            color_t color = emu->ppu.color_buffer[i];
            draw_display(&io->display, position, color);
        }
    }
    if (is_debug_io(io)) {
        debug_io(io, emu);
    }

    refresh_display(&io->display);
    return !io->input.quit;
}