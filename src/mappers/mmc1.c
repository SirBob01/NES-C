#include "./mmc1.h"

void create_mmc1(mmc1_t *mapper) {
    mapper->shift_register = 0;
    mapper->write_counter = 0;
}

void destroy_mmc1(mmc1_t *mapper) {}

unsigned char read_cpu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address) {
    // if (address < 0x6000) {
    //     return 0;
    // } else if (address < 0x8000) {
    //     return rom->prg_ram[address - 0x6000];
    // } else {
    //     return rom->prg_rom[address - 0x8000];
    // }
}

unsigned char read_ppu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address) {
    // if (address < 0x2000) {
    //     return rom->chr[address];
    // } else {
    //     return 0;
    // }
}

void write_cpu_mmc1(mmc1_t *mapper,
                    rom_t *rom,
                    address_t address,
                    unsigned char value) {
    // if (value & 0x80) {
    //     mapper->shift_register = 0;
    // } else {
    // mapper->write_counter++;
    // mapper->shift_register |= (value & 0x01) << mapper->write_counter;
    // }
}

void write_ppu_mmc1(mmc1_t *mapper,
                    rom_t *rom,
                    address_t address,
                    unsigned char value) {
    fprintf(stderr, "Error: MMC1 does not support PPU writes\n");
}
