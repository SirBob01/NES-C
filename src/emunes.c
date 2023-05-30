#include "./init.h"

int main(int argc, char **argv) {
    // Load a ROM into memory
    rom_t rom = parse_args(argc, argv);
    if (rom.buffer == NULL) {
        return 1;
    }

    // TODO: Execute instructions in main loop

    // Cleanup
    free_rom(&rom);
    return 0;
}