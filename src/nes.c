#include "./nes.h"

void print_usage() { printf("Usage: nesc %s <input_file>\n", ARG_INPUT_FILE); }

void parse_args(emulator_t *emu, int argc, char **argv) {
    // Verify arguments
    if (argc < 3) {
        print_usage();
        exit(1);
    }
    if (strcmp(argv[1], ARG_INPUT_FILE) != 0) {
        print_usage();
        exit(1);
    }

    create_emulator(emu, argv[2]);
    if (argc == 4) {
        emu->cpu.pc = strtol(argv[3], NULL, 16);
    }
}

int main(int argc, char **argv) {
    // Initialize string buffer for debugging
    char strbuf[1024];

    // Boot up the emulator
    emulator_t emu;
    parse_args(&emu, argc, argv);
    if (emu.rom.data.buffer == NULL || emu.rom.header.type == NES_INVALID) {
        exit(1);
    }

    // Setup host machine IO
    io_t io;
    create_io(&io, &emu);

    // Print ROM information
    read_state_rom(&emu.rom, strbuf, sizeof(strbuf));
    puts(strbuf);

    // Emulate and refresh device IO every frame
    while (true) {
        bool emu_state = true;
        while (emu_state) {
            unsigned prev_frame = emu.frames;
            emu_state = update_emulator(&emu);
            unsigned curr_frame = emu.frames;

            if (curr_frame > prev_frame) {
                break;
            }
        }

        bool io_state = refresh_io(&io, &emu);
        if (!emu_state || !io_state) {
            break;
        }
    }

    // Cleanup
    destroy_io(&io);
    destroy_emulator(&emu);
    return 0;
}