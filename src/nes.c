#include "./nes.h"

void print_usage() { printf("Usage: nesc %s <input_file>\n", ARG_INPUT_FILE); }

emulator_t *parse_args(int argc, char **argv) {
    // Verify arguments
    if (argc < 3) {
        print_usage();
        exit(1);
    }
    if (strcmp(argv[1], ARG_INPUT_FILE) != 0) {
        print_usage();
        exit(1);
    }

    emulator_t *emu = create_emulator(argv[2]);
    if (argc == 4) {
        address_t pc = strtol(argv[3], NULL, 16);
        emu->cpu->pc = pc;
    }
    return emu;
}

int main(int argc, char **argv) {
    // Initialize string buffer for debugging
    char strbuf[1024];

    // Boot up the emulator
    emulator_t *emu = parse_args(argc, argv);
    io_t *io = create_io(emu);
    if (emu->rom->data.buffer == NULL || emu->rom->header.type == NES_INVALID) {
        exit(1);
    }

    // Print ROM information
    read_state_rom(emu->rom, strbuf, sizeof(strbuf));
    puts(strbuf);

    // Run the emulator until it finishes
    while (true) {
        bool emu_state = update_emulator(emu);
        bool io_state = refresh_io(io, emu);
        if (!emu_state || !io_state) {
            break;
        }
    }

    // Cleanup
    destroy_io(io);
    destroy_emulator(emu);
    return 0;
}