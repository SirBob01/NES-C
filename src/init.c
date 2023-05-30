#include "./init.h"

void print_usage() {
    printf("Usage: emunes %s <rom_filepath>\n", ARG_INPUT_FILE);
}

rom_t parse_args(int argc, char **argv) {
    rom_t rom;
    rom.buffer = NULL;
    rom.size = 0;

    // Verify argument count
    if (argc < 3) {
        print_usage();
        return rom;
    }

    // Verify correct argument list
    if (strcmp(argv[1], ARG_INPUT_FILE) != 0) {
        print_usage();
        return rom;
    }

    // Open file
    FILE *file = fopen(argv[2], "rb");
    if (file == NULL) {
        printf("Error: Could not open file \"%s\"\n", argv[2]);
        return rom;
    }

    // Get length of file
    fseek(file, 0, SEEK_END);
    rom.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate ROM memory
    rom.buffer = (char *)malloc(rom.size);
    if (rom.buffer == NULL) {
        printf("Error: Could not allocate memory for ROM\n");
        return rom;
    }

    // Read file into buffer
    unsigned long i = 0;
    char c = 0;
    while ((c = getc(file)) != EOF) {
        assert(i < rom.size);
        rom.buffer[i++] = c;
    }

    // Cleanup and return
    fclose(file);
    return rom;
}

void free_rom(rom_t *rom) { free(rom->buffer); }
