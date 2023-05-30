#include "./emunes.h"

void print_usage() {
    printf("Usage: emunes %s <input_file>\n", ARG_INPUT_FILE);
}

void print_rom(rom_t *rom) {
    printf("ROM loaded successfully...\n");
    if (rom->header.type == NES_1) {
        printf("ROM type: NES 1\n");
    } else if (rom->header.type == NES_2) {
        printf("ROM type: NES 2\n");
    } else {
        // This should never happen
        printf("ROM type: Unknown\n");
    }
    printf("PRG ROM %lu\n", rom->header.prg_rom_size);
    printf("CHR ROM %lu\n", rom->header.chr_rom_size);
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