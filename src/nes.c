#include "./nes.h"

void print_usage() { printf("Usage: nesc %s <input_file>\n", ARG_INPUT_FILE); }

void print_rom(rom_t *rom) {
    printf("ROM loaded successfully...\n");
    printf("* ROM type: %s\n", rom->header.type == NES_1 ? "NES 1" : "NES 2");
    printf("* PRG ROM: %lu\n", rom->header.prg_rom_size);
    printf("* CHR ROM: %lu\n", rom->header.chr_rom_size);
    printf("* PRG ROM Offset: 0x%02X\n",
           (unsigned)(get_prg_rom(rom) - rom->data.buffer));
    printf("* CHR ROM Offset: 0x%02X\n",
           (unsigned)(get_chr_rom(rom) - rom->data.buffer));
    printf("* Mirroring: %s\n",
           rom->header.mirroring == MIRROR_HORIZONTAL ? "Horizontal"
                                                      : "Vertical");
    printf("* Battery: %s\n", rom->header.battery ? "Yes" : "No");
    printf("* Trainer: %s\n", rom->header.trainer ? "Yes" : "No");
    printf("* Four-screen: %s\n", rom->header.four_screen ? "Yes" : "No");
    printf("* Console type: %s\n",
           rom->header.console_type == CONSOLE_NES          ? "NES"
           : rom->header.console_type == CONSOLE_VS         ? "VS"
           : rom->header.console_type == CONSOLE_PLAYCHOICE ? "PlayChoice"
                                                            : "Extended");
    printf("* Mapper number: %d\n", rom->header.mapper);
}

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
    // Boot up the emulator
    emulator_t *emu = parse_args(argc, argv);
    io_t *io = create_io(emu);
    if (emu->rom->data.buffer == NULL || emu->rom->header.type == NES_INVALID) {
        return 1;
    }

    // Print ROM information
    print_rom(emu->rom);

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