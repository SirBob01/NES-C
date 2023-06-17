#include "./io.h"

io_t *create_io(emulator_t *emu) {
    io_t *io = (io_t *)malloc(sizeof(io_t));
    io->emu = emu;
    io->display = create_display(256, 240, "SirBob's NES-C");
    io->audio = create_audio(emu->apu->buffer);
    io->input = create_input();

#ifndef NDEBUG
    io->pattern_table = create_display(128, 256, "Pattern Tables");
#endif
    return io;
}

void destroy_io(io_t *io) {
    destroy_display(io->display);
    destroy_input(io->input);
    destroy_audio(io->audio);
    free(io);
}

void debug_pattern_tables(display_t *display, rom_t *rom) {
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
                render_pixel(display, position, color);
            }
        }
    }
}

bool refresh_io(io_t *io, emulator_t *emu) {
    poll_input(io->input);

    // Draw the color buffer from the PPU.
    for (unsigned x = 0; x < io->display->size.x; x++) {
        for (unsigned y = 0; y < io->display->size.y; y++) {
            unsigned i = x + y * io->display->size.x;
            color_t pixel = emu->ppu->color_buffer[i];
            vec2_t position = {x, y};
            render_pixel(io->display, position, pixel);
        }
    }
    refresh_display(io->display);

#ifndef NDEBUG
    // Debug draw the pattern tables.
    debug_pattern_tables(io->pattern_table, emu->rom);
    refresh_display(io->pattern_table);
#endif

    return !io->input->quit;
}