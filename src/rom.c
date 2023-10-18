#include "./rom.h"

void load_rom(rom_t *rom, const char *path) {
    rom->data.buffer = NULL;
    rom->data.size = 0;

    // Open file
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file \"%s\"\n", path);
        exit(1);
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    unsigned long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size < 16) {
        fprintf(stderr, "Error: ROM is too small\n");
        fclose(file);
        exit(1);
    }

    // Read the contents of the file
    memory_t file_buffer = allocate_memory(size);
    unsigned read = fread(file_buffer.buffer, 1, file_buffer.size, file);
    if (read != file_buffer.size) {
        fprintf(stderr, "Error: Could not read file \"%s\"\n", path);
        fclose(file);
        exit(1);
    }

    // Parse the iNES file
    rom->header = get_header_rom(file_buffer.buffer);
    rom->data =
        allocate_memory(rom->header.prg_rom_size + rom->header.prg_ram_size +
                        rom->header.chr_rom_size + rom->header.chr_ram_size +
                        rom->header.trainer_size);

    // Copy Trainer and ROM data to the buffer
    unsigned trainer_offset = 0x10;
    unsigned prg_rom_offset = trainer_offset + rom->header.trainer_size;
    unsigned chr_rom_offset = prg_rom_offset + rom->header.prg_rom_size;
    memcpy(get_trainer_rom(rom),
           file_buffer.buffer + trainer_offset,
           rom->header.trainer_size);
    memcpy(get_prg_rom(rom),
           file_buffer.buffer + prg_rom_offset,
           rom->header.prg_rom_size);
    memcpy(get_chr_rom(rom),
           file_buffer.buffer + chr_rom_offset,
           rom->header.chr_rom_size);

    // Cleanup and return
    free_memory(&file_buffer);
    fclose(file);
}

void unload_rom(rom_t *rom) { free_memory(&rom->data); }

rom_header_t get_header_rom(const unsigned char *buffer) {
    rom_header_t header;

    // Determine format type
    header.type = NES_INVALID;
    if (buffer[0] == 'N' && buffer[1] == 'E' && buffer[2] == 'S' &&
        buffer[3] == 0x1a) {
        header.type = NES_1;
    } else {
        fprintf(stderr, "Error: Invalid ROM\n");
        exit(1);
    }
    if ((buffer[7] & 0x0c) == 0x08) {
        header.type = NES_2;
    }

    // Metadata
    header.mirroring = (buffer[6] & 0x1) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL;
    header.battery = buffer[6] & 0x2;
    header.four_screen = buffer[6] & 0x8;
    header.mapper = (buffer[7] & 0xf0) | ((buffer[6] & 0xf0) >> 4);

    // Memory segment sizes
    header.trainer_size = buffer[6] & 0x4 ? 0x200 : 0;
    header.prg_rom_size = buffer[4] * (1 << 14);
    header.prg_ram_size = buffer[8] * (1 << 13);
    header.chr_rom_size = buffer[5] * (1 << 13);
    if (header.prg_ram_size == 0) {
        header.prg_ram_size = 0x2000;
    }
    if (header.chr_rom_size == 0) {
        header.chr_ram_size = 0x2000;
    }

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

unsigned char *get_trainer_rom(rom_t *rom) { return rom->data.buffer; }

unsigned char *get_prg_rom(rom_t *rom) {
    return get_trainer_rom(rom) + rom->header.trainer_size;
}

unsigned char *get_prg_ram(rom_t *rom) {
    return get_prg_rom(rom) + rom->header.prg_rom_size;
}

unsigned char *get_chr_rom(rom_t *rom) {
    return get_prg_ram(rom) + rom->header.prg_ram_size;
}

unsigned char *get_chr_ram(rom_t *rom) {
    return get_chr_rom(rom) + rom->header.chr_rom_size;
}

void read_state_rom(rom_t *rom, char *buffer, unsigned buffer_size) {
    snprintf(buffer,
             buffer_size,
             "iNES Cartridge:\n"
             "* iNES type: %s\n"
             "* iNES Size: %lu\n"
             "* Trainer Size: %u\n"
             "* PRG ROM Size: %u\n"
             "* PRG RAM Size: %u\n"
             "* CHR ROM Size: %u\n"
             "* CHR RAM Size: %u\n"
             "* Mirroring: %s\n"
             "* Battery: %s\n"
             "* Four-screen: %s\n"
             "* Console type: %s\n"
             "* Mapper number: %d\n",
             rom->header.type == NES_1 ? "NES 1" : "NES 2",
             rom->data.size,
             rom->header.trainer_size,
             rom->header.prg_rom_size,
             rom->header.prg_ram_size,
             rom->header.chr_rom_size,
             rom->header.chr_ram_size,
             rom->header.mirroring == MIRROR_HORIZONTAL ? "Horizontal"
                                                        : "Vertical",
             rom->header.battery ? "Yes" : "No",
             rom->header.four_screen ? "Yes" : "No",
             rom->header.console_type == CONSOLE_NES          ? "NES"
             : rom->header.console_type == CONSOLE_VS         ? "VS"
             : rom->header.console_type == CONSOLE_PLAYCHOICE ? "PlayChoice"
                                                              : "Extended",
             rom->header.mapper);
}