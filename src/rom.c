#include "./rom.h"

rom_t load_rom(const char *path) {
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
    fread(rom.data.buffer, 1, rom.data.size, file);

    // Parse the header
    rom.header = get_rom_header(rom.data.buffer);

    // Cleanup and return
    fclose(file);
    return rom;
}

rom_header_t get_rom_header(const unsigned char *buffer) {
    rom_header_t header;

    // Determine format type
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

    // Metadata
    header.prg_rom_size = buffer[4] * (1 << 14);
    header.chr_rom_size = buffer[5] * (1 << 13);
    header.mirroring = (buffer[6] & 0x1) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL;
    header.battery = buffer[6] & 0x2;
    header.trainer = buffer[6] & 0x4;
    header.four_screen = buffer[6] & 0x8;
    header.mapper = (buffer[7] & 0xf0) | ((buffer[6] & 0xf0) >> 4);

    // Console type
    switch (buffer[7] & 0x03) {
    case 0:
        header.console_type = CONSOLE_NES;
        break;
    case 1:
        header.console_type = CONSOLE_VS;
        break;
    case 2:
        header.console_type = CONSOLE_PLAYCHOICE;
        break;
    case 3:
    default:
        header.console_type = CONSOLE_EXTENDED;
        break;
    }
    return header;
}

unsigned char *get_trainer(rom_t *rom) {
    unsigned char *base = rom->data.buffer;
    return base + 0x10;
}

unsigned char *get_prg_rom(rom_t *rom) {
    return get_trainer(rom) + (rom->header.trainer ? 0x200 : 0);
}

unsigned char *get_chr_rom(rom_t *rom) {
    return get_prg_rom(rom) + rom->header.prg_rom_size;
}

void free_rom(rom_t *rom) { free(rom->data.buffer); }