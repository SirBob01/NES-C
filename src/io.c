#include "./io.h"

void create_io(io_t *io, emulator_t *emu) {
    io->emu = emu;

#ifndef NDEBUG
    create_display(&io->pattern_table, 128, 256, "Pattern Tables");
#endif
    create_display(&io->display, 256, 240, "SirBob's NES-C");
    create_audio(&io->audio, &emu->apu.buffer);
    create_input(&io->input);
}

void destroy_io(io_t *io) {
    destroy_display(&io->display);
    destroy_input(&io->input);
    destroy_audio(&io->audio);
}

void debug_pattern_tables_io(display_t *display, rom_t *rom) {
    unsigned char *chr_rom = get_chr_rom(rom);
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
                draw_display(display, position, color);
            }
        }
    }
}

bool refresh_io(io_t *io, emulator_t *emu) {
    poll_input(&io->input);

    // Draw the color buffer from the PPU.
    for (unsigned x = 0; x < io->display.size.x; x++) {
        for (unsigned y = 0; y < io->display.size.y; y++) {
            unsigned i = x + y * PPU_LINEDOTS;
            color_t pixel = emu->ppu.color_buffer[i];
            vec2_t position = {x, y};
            draw_display(&io->display, position, pixel);
        }
    }
    refresh_display(&io->display);

#ifndef NDEBUG
    // Debug draw the pattern tables.
    debug_pattern_tables_io(&io->pattern_table, &emu->rom);
    refresh_display(&io->pattern_table);
#endif

    return !io->input.quit;
}