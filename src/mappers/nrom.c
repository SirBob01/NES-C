#include "./nrom.h"

unsigned char *get_cpu_memory_nrom(rom_t *rom, address_t address) {
    bool nrom128 = rom->header.prg_rom_size == 0x4000;
    address_t rom_addr = address - 0x8000;
    return get_prg_rom(rom) + rom_addr % (nrom128 ? 0x4000 : 0x8000);
}

unsigned char *get_ppu_memory_nrom(rom_t *rom, address_t address) {
    return get_chr_rom(rom) + address;
}

unsigned char read_cpu_nrom(rom_t *rom, address_t address) {
    return *get_cpu_memory_nrom(rom, address);
}

unsigned char read_ppu_nrom(rom_t *rom, address_t address) {
    return *get_ppu_memory_nrom(rom, address);
}

void write_cpu_nrom(rom_t *rom, address_t address, unsigned char value) {
    fprintf(stderr, "Error: Attempted to write to CPU memory in NROM\n");
    exit(1);
}

void write_ppu_nrom(rom_t *rom, address_t address, unsigned char value) {
    // If CHR ROM is not present, write to CHR RAM
    if (rom->header.chr_rom_size == 0) {
        *get_ppu_memory_nrom(rom, address) = value;
    }
}