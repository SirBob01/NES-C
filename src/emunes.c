#define ARG_INPUT_FILE "-i"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./memory.h"
#include "./rom.h"

void print_usage() {
    printf("Usage: emunes %s <input_file>\n", ARG_INPUT_FILE);
}

memory_t parse_args(int argc, char **argv) {
    memory_t rom;
    rom.buffer = NULL;
    rom.size = 0;

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
    memory_t rom = parse_args(argc, argv);
    if (rom.buffer == NULL) {
        return 1;
    }

    // TODO: Execute instructions in main loop

    // Cleanup
    free_memory(&rom);
    return 0;
}