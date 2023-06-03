#include "./mapper.h"

unsigned char *nrom_mapper(cpu_t *cpu, rom_t *rom, address_t address) {
    bool nrom128 = rom->header.prg_rom_size == 0x4000;
    address_t rom_offset = 0x8000;
    if (address >= rom_offset) {
        address_t rom_addr = address - rom_offset;
        return get_prg_rom(rom) + (rom_addr % (nrom128 * 0x4000));
    } else {
        return cpu->memory.buffer + address;
    }
}
