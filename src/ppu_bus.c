#include "./ppu_bus.h"
#include "./cpu_bus.h"

void create_ppu_bus(ppu_bus_t *bus, rom_t *rom, mapper_t *mapper) {
    bus->rom = rom;
    bus->mapper = mapper;
    memset(bus->memory, 0, PPU_RAM_SIZE);
}

address_t mirror_address_ppu_bus(address_t address, rom_mirroring_t mirroring) {
    if (address >= PPU_MAP_NAMETABLE_MIRROR) {
        address -= 0x1000;
    }
    if (address >= PPU_MAP_NAMETABLE_0) {
        // Mirrored nametable regions
        switch (mirroring) {
        case MIRROR_VERTICAL:
            return PPU_MAP_NAMETABLE_0 + (address & 0x7FF);
        case MIRROR_HORIZONTAL: {
            if (address < PPU_MAP_NAMETABLE_2) {
                return PPU_MAP_NAMETABLE_0 + (address & 0x3FF);
            } else {
                return PPU_MAP_NAMETABLE_2 + (address & 0x3FF);
            }
        }
        }
    }
    return address;
}

unsigned char read_ppu_bus(ppu_bus_t *bus, address_t address) {
    if (address >= PPU_MAP_NAMETABLE_0) {
        address = mirror_address_ppu_bus(address, bus->rom->header.mirroring);
        return bus->memory[address];
    } else {
        return read_ppu_mapper(bus->mapper, address);
    }
}

void write_ppu_bus(ppu_bus_t *bus, address_t address, unsigned char value) {
    if (address >= PPU_MAP_NAMETABLE_0) {
        address = mirror_address_ppu_bus(address, bus->rom->header.mirroring);
        bus->memory[address] = value;
    } else {
        write_ppu_mapper(bus->mapper, address, value);
    }
}