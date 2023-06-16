#include "./io.h"

io_t create_io(emulator_t *emu) {
    io_t io;
    io.emu = emu;
    io.audio = create_audio(&emu->apu.buffer);
    io.input = create_input();
    io.display = create_display(256, 240, "SirBob's NES-C");
    return io;
}

void destroy_io(io_t *io) {
    destroy_display(&io->display);
    destroy_audio(&io->audio);
}

bool refresh_io(io_t *io) {
    poll_input(&io->input);
    refresh_display(&io->display);
    return !io->input.quit;
}