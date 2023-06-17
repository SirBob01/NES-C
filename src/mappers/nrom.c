#include "./nrom.h"

unsigned char *nrom_cpu(cpu_t *cpu, address_t address) {
    bool nrom128 = cpu->rom->header.prg_rom_size == 0x4000;
    address_t rom_offset = 0x8000;
    if (address >= rom_offset) {
        address_t rom_addr = address - rom_offset;
        return get_prg_rom(cpu->rom) + (rom_addr % (nrom128 * 0x4000));
    } else {
        return cpu->memory.buffer + address;
    }
}
