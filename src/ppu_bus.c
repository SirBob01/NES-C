#include "./ppu_bus.h"
#include "./mappers/nrom.h"

void create_ppu_bus(ppu_bus_t *bus, rom_t *rom) {
    bus->rom = rom;
    memset(bus->memory, 0, CPU_RAM_SIZE);
}

address_t mirror_address_ppu_bus(address_t address, rom_mirroring_t mirroring) {
    if (address < PPU_MAP_NAMETABLE_MIRROR && address >= PPU_MAP_NAMETABLE_0) {
        // Mirrored nametable regions
        switch (mirroring) {
        case MIRROR_VERTICAL:
            return PPU_MAP_NAMETABLE_0 +
                   ((address - PPU_MAP_NAMETABLE_0) % 0x800);
        case MIRROR_HORIZONTAL: {
            if (address < PPU_MAP_NAMETABLE_2) {
                return PPU_MAP_NAMETABLE_0 +
                       ((address - PPU_MAP_NAMETABLE_0) % 0x400);
            } else {
                return PPU_MAP_NAMETABLE_2 +
                       ((address - PPU_MAP_NAMETABLE_2) % 0x400);
            }
        }
        }
    } else if (address >= PPU_MAP_PALETTE && address < PPU_MAP_PALETTE_MIRROR) {
        // Mirrored palette table regions
        switch (address) {
        case 0x3f10:
            return 0x3f00;
        case 0x3f14:
            return 0x3f04;
        case 0x3f18:
            return 0x3f08;
        case 0x3f1c:
            return 0x3f0c;
        default:
            return address;
        }
    } else if (address >= PPU_MAP_PALETTE_MIRROR) {
        // Mirrored palette table regions
        address_t norm = PPU_MAP_PALETTE + ((address - PPU_MAP_PALETTE) % 0x20);
        switch (norm) {
        case 0x3f10:
            return 0x3f00;
        case 0x3f14:
            return 0x3f04;
        case 0x3f18:
            return 0x3f08;
        case 0x3f1c:
            return 0x3f0c;
        default:
            return norm;
        }
    }
    return address;
}

unsigned char *apply_memory_mapper_ppu_bus(ppu_bus_t *bus, address_t address) {
    switch (bus->rom->header.mapper) {
    case 0:
        return nrom_ppu(bus, address);
    default:
        fprintf(stderr,
                "Error: Unsupported PPU mapper %d\n",
                bus->rom->header.mapper);
        exit(1);
    }
}

unsigned char *get_memory_ppu_bus(ppu_bus_t *bus, address_t address) {
    if (address >= PPU_MAP_NAMETABLE_0) {
        rom_mirroring_t mirroring = bus->rom->header.mirroring;
        return bus->memory + mirror_address_ppu_bus(address, mirroring);
    }
    return apply_memory_mapper_ppu_bus(bus, address);
}