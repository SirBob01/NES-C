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
    }
    destroy_display(&io->display);
    destroy_input(&io->input);
    destroy_audio(&io->audio);
}

bool is_debug_io(io_t *io) {
    // Check if pattern tables are being rendered
    return !is_free_memory(&io->pattern_table.bitmap);
}

void set_debug_io(io_t *io, bool debug) {
    if (is_debug_io(io) == debug) return;
    if (debug) {
        create_display(&io->pattern_table, 128, 256, "Pattern Tables");
    } else {
        destroy_display(&io->pattern_table);
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
                    (i % 16) * 8 + x,
                    (unsigned)(i / 16) * 8 + y,
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