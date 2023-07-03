#include "./nrom.h"

unsigned char *nrom_cpu(cpu_bus_t *bus, address_t address) {
    bool nrom128 = bus->rom->header.prg_rom_size == 0x4000;
    address_t rom_offset = 0x8000;
    if (address >= rom_offset) {
        address_t rom_addr = address - rom_offset;
        return get_prg_rom(bus->rom) + rom_addr % (nrom128 ? 0x4000 : 0x8000);
    } else {
        return bus->memory + address;
    }
}

unsigned char *nrom_ppu(ppu_bus_t *bus, address_t address) {
    return get_chr_rom(bus->rom) + address;
}