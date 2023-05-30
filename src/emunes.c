#include "./emunes.h"

void print_usage() {
    printf("Usage: emunes %s <input_file>\n", ARG_INPUT_FILE);
}

void print_rom(rom_t *rom) {
    printf("ROM loaded successfully...\n");
    printf("* ROM type: %s\n", rom->header.type == NES_1 ? "NES 1" : "NES 2");
    printf("* PRG ROM %lu\n", rom->header.prg_rom_size);
    printf("* CHR ROM %lu\n", rom->header.chr_rom_size);
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
    printf("* Submapper number: %d\n", rom->header.submapper);
}

rom_t parse_args(int argc, char **argv) {
    rom_t rom;
    rom.data.buffer = NULL;

    // Verify arguments
    if (argc < 3) {
        print_usage();
        return rom;
    }
    if (strcmp(argv[1], ARG_INPUT_FILE) != 0) {
        print_usage();
        return rom;
    }
    return load_rom(argv[2]);
}

int main(int argc, char **argv) {
    // Load a ROM into memory
    rom_t rom = parse_args(argc, argv);
    if (rom.data.buffer == NULL || rom.header.type == NES_INVALID) {
        return 1;
    }

    // Print ROM information
    print_rom(&rom);

    // Run main program loop

    // Cleanup
    free_rom(&rom);
    return 0;
}