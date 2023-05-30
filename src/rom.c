#include "./rom.h"
#include "memory.h"

memory_t load_rom(char *path) {
    memory_t rom;
    rom.buffer = NULL;
    rom.size = 0;

    // Open file
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file \"%s\"\n", path);
        return rom;
    }

    // Allocate memory
    fseek(file, 0, SEEK_END);
    rom = allocate_memory(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Write the contents of the file to the buffer
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