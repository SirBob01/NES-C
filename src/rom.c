#include "./rom.h"

rom_t load_rom(char *path) {
    rom_t rom;
    rom.data.buffer = NULL;
    rom.data.size = 0;

    // Open file
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file \"%s\"\n", path);
        return rom;
    }

    // Allocate memory
    fseek(file, 0, SEEK_END);
    rom.data = allocate_memory(ftell(file));
    fseek(file, 0, SEEK_SET);

    // Write the contents of the file to the buffer
    unsigned long i = 0;
    char c = 0;
    while ((c = getc(file)) != EOF) {
        assert(i < rom.data.size);
        rom.data.buffer[i++] = c;
    }

    // Parse the header
    rom.header = get_rom_header(rom.data.buffer);

    // Cleanup and return
    fclose(file);
    return rom;
}

rom_header_t get_rom_header(const char *buffer) {
    rom_header_t header;
    header.type = NES_INVALID;

    if (buffer[0] == 'N' && buffer[1] == 'E' && buffer[2] == 'S' &&
        buffer[3] == 0x1A) {
        header.type = NES_1;
    } else {
        fprintf(stderr, "Error: Invalid ROM\n");
        return header;
    }
    if ((buffer[7] & 0x0c) == 0x08) {
        header.type = NES_2;
    }
    header.prg_rom_size = buffer[4] * (1 << 14);
    header.chr_rom_size = buffer[5] * (1 << 13);
    return header;
}

void free_rom(rom_t *rom) { free(rom->data.buffer); }