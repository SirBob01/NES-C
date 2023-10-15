#include "./cpu_bus.h"

void create_cpu_bus(cpu_bus_t *bus,
                    rom_t *rom,
                    mapper_t *mapper,
                    apu_t *apu,
                    ppu_t *ppu) {
    bus->rom = rom;
    bus->mapper = mapper;
    bus->apu = apu;
    bus->ppu = ppu;
    memset(bus->memory, 0, CPU_RAM_SIZE);
}

address_t mirror_cpu_bus(address_t address, unsigned long prg_ram_size) {
    if (address < CPU_MAP_PPU_REG) {
        // Mirrored RAM region
        return CPU_MAP_START + ((address - CPU_MAP_START) % 0x800);
    } else if (address < CPU_MAP_APU_IO) {
        // Mirrored PPU register memory
        return CPU_MAP_PPU_REG + ((address - CPU_MAP_PPU_REG) % 0x8);
    } else if (address >= CPU_MAP_RAM) {
        // Mirrored PRG RAM memory
        return CPU_MAP_RAM + ((address - CPU_MAP_RAM) % prg_ram_size);
    }
    return address;
}

void oamdma_cpu_bus(cpu_bus_t *bus, unsigned char value) {
    address_t dma_addr = value << 8;
    for (unsigned i = 0; i < 256; i++) {
        // Get
        unsigned char data = read_cpu_bus(bus, dma_addr + i);

        // Put
        write_cpu_bus(bus, PPU_REG_OAMDATA, data);
    }
}

unsigned char read_cpu_bus(cpu_bus_t *bus, address_t address) {
    if (address >= CPU_MAP_ROM) {
        return read_cpu_mapper(bus->mapper, address);
    } else {
        address = mirror_cpu_bus(address, bus->rom->header.prg_ram_size);
        switch (address) {
        case PPU_REG_STATUS:
            return read_status_ppu(bus->ppu);
        case PPU_REG_OAMDATA:
            return read_oamdata_ppu(bus->ppu);
        case PPU_REG_DATA:
            return read_data_ppu(bus->ppu);
        case PPU_REG_CTRL:
        case PPU_REG_MASK:
        case PPU_REG_ADDR:
        case PPU_REG_SCROLL:
        case PPU_REG_OAMADDR:
        case PPU_REG_OAMDMA:
            return bus->ppu->io_databus;
        // TODO: Handle APU, controller input registers
        default:
            return bus->memory[address];
        }
    }
}

void write_cpu_bus(cpu_bus_t *bus, address_t address, unsigned char value) {
    if (address >= CPU_MAP_ROM) {
        write_cpu_mapper(bus->mapper, address, value);
    } else {
        address = mirror_cpu_bus(address, bus->rom->header.prg_ram_size);
        switch (address) {
        case PPU_REG_CTRL:
            write_ctrl_ppu(bus->ppu, value);
            break;
        case PPU_REG_MASK:
            write_mask_ppu(bus->ppu, value);
            break;
        case PPU_REG_OAMDATA:
            write_oamdata_ppu(bus->ppu, value);
            break;
        case PPU_REG_SCROLL:
            write_scroll_ppu(bus->ppu, value);
            break;
        case PPU_REG_ADDR:
            write_addr_ppu(bus->ppu, value);
            break;
        case PPU_REG_DATA:
            write_data_ppu(bus->ppu, value);
            break;
        case PPU_REG_OAMADDR:
            write_oamaddr_ppu(bus->ppu, value);
            break;
        case PPU_REG_OAMDMA:
            oamdma_cpu_bus(bus, value);
        case PPU_REG_STATUS:
            bus->ppu->io_databus = value;
            break;
        // TODO: Handle APU, controller input registers
        default:
            bus->memory[address] = value;
            break;
        }
    }
}

unsigned
read_string_cpu_bus(cpu_bus_t *bus, address_t address, char *dst, unsigned n) {
    char c = read_cpu_bus(bus, address);
    unsigned length = 0;
    while (c && length < n - 1) {
        dst[length++] = c;
        c = read_cpu_bus(bus, ++address);
    }
    dst[length] = 0;
    return length;
}
