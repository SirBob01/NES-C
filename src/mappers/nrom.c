#include "./nrom.h"

unsigned char read_cpu_nrom(rom_t *rom, address_t address) {
    if (address >= 0x8000) {
        address_t mask = rom->header.prg_rom_size - 1;
        return get_prg_rom(rom)[address & mask];
    } else if (address >= 0x6000) {
        address_t mask = rom->header.prg_ram_size - 1;
        return get_prg_ram(rom)[address & mask];
    } else {
        return 0;
    }
}

unsigned char read_ppu_nrom(rom_t *rom, address_t address) {
    return get_chr_rom(rom)[address];
}

void write_cpu_nrom(rom_t *rom, address_t address, unsigned char value) {
    if (address >= 0x6000 && address < 0x8000) {
        address_t mask = rom->header.prg_ram_size - 1;
        get_prg_ram(rom)[address & mask] = value;
    }
}

void write_ppu_nrom(rom_t *rom, address_t address, unsigned char value) {
    if (rom->header.chr_ram_size > 0) {
        get_chr_rom(rom)[address] = value;
    }
}