#include "./cpu_bus.h"
#include "./mappers/nrom.h"

void create_cpu_bus(cpu_bus_t *bus, rom_t *rom, apu_t *apu, ppu_t *ppu) {
    bus->rom = rom;
    bus->apu = apu;
    bus->ppu = ppu;
}

address_t mirror_address_cpu_bus(address_t address) {
    if (address < CPU_MAP_PPU_REG) {
        // Mirrored RAM region
        return CPU_MAP_RAM + ((address - CPU_MAP_RAM) % 0x800);
    } else if (address < CPU_MAP_APU_IO) {
        // Mirrored PPU register memory
        return CPU_MAP_PPU_REG + ((address - CPU_MAP_PPU_REG) % 0x8);
    }
    return address;
}

unsigned char *apply_memory_mapper_cpu_bus(cpu_bus_t *bus, address_t address) {
    switch (bus->rom->header.mapper) {
    case 0:
        return nrom_cpu(bus, address);
    default:
        fprintf(stderr,
                "Error: Unsupported CPU mapper %d\n",
                bus->rom->header.mapper);
        exit(1);
    }
}

unsigned char *get_memory_cpu_bus(cpu_bus_t *bus, address_t address) {
    if (address >= CPU_MAP_CARTRIDGE) {
        return apply_memory_mapper_cpu_bus(bus, address);
    }
    address_t norm_address = mirror_address_cpu_bus(address);
    switch (norm_address) {
    case PPU_REG_CTRL:
        return &bus->ppu->ctrl;
    case PPU_REG_MASK:
        return &bus->ppu->mask;
    case PPU_REG_STATUS:
        return &bus->ppu->status;
    case PPU_REG_OAMADDR:
        return &bus->ppu->oam_addr;
    case PPU_REG_OAMDATA:
        return &bus->ppu->oam_data;
    case PPU_REG_SCROLL:
        return &bus->ppu->scroll;
    case PPU_REG_ADDR:
        return &bus->ppu->addr;
    case PPU_REG_DATA:
        return &bus->ppu->data;
    case PPU_REG_OAMDMA:
        return &bus->ppu->oam_dma;
    case APU_REG_PULSE1_0:
        return &bus->apu->channel_registers.pulse1[0];
    case APU_REG_PULSE1_1:
        return &bus->apu->channel_registers.pulse1[1];
    case APU_REG_PULSE1_2:
        return &bus->apu->channel_registers.pulse1[2];
    case APU_REG_PULSE1_3:
        return &bus->apu->channel_registers.pulse1[3];
    case APU_REG_PULSE2_0:
        return &bus->apu->channel_registers.pulse2[0];
    case APU_REG_PULSE2_1:
        return &bus->apu->channel_registers.pulse2[1];
    case APU_REG_PULSE2_2:
        return &bus->apu->channel_registers.pulse2[2];
    case APU_REG_PULSE2_3:
        return &bus->apu->channel_registers.pulse2[3];
    case APU_REG_TRIANGLE_0:
        return &bus->apu->channel_registers.triangle[0];
    case APU_REG_TRIANGLE_1:
        return &bus->apu->channel_registers.triangle[1];
    case APU_REG_TRIANGLE_2:
        return &bus->apu->channel_registers.triangle[2];
    case APU_REG_TRIANGLE_3:
        return &bus->apu->channel_registers.triangle[3];
    case APU_REG_NOISE_0:
        return &bus->apu->channel_registers.noise[0];
    case APU_REG_NOISE_1:
        return &bus->apu->channel_registers.noise[1];
    case APU_REG_NOISE_2:
        return &bus->apu->channel_registers.noise[2];
    case APU_REG_NOISE_3:
        return &bus->apu->channel_registers.noise[3];
    case APU_REG_DMC_0:
        return &bus->apu->channel_registers.dmc[0];
    case APU_REG_DMC_1:
        return &bus->apu->channel_registers.dmc[1];
    case APU_REG_DMC_2:
        return &bus->apu->channel_registers.dmc[2];
    case APU_REG_DMC_3:
        return &bus->apu->channel_registers.dmc[3];
    case APU_REG_STATUS:
        return &bus->apu->status;
    case APU_REG_FRAME_COUNTER:
        return &bus->apu->frame_counter;
    default:
        return bus->memory + norm_address;
    }
}

unsigned char read_byte_cpu_bus(cpu_bus_t *bus, address_t address) {
    return *get_memory_cpu_bus(bus, address);
}

unsigned short read_short_cpu_bus(cpu_bus_t *bus, address_t address) {
    unsigned char a0 = read_byte_cpu_bus(bus, address);
    unsigned char a1 = read_byte_cpu_bus(bus, address + 1);
    return a0 | (a1 << 8);
}

unsigned short read_short_zp_cpu_bus(cpu_bus_t *bus, unsigned char address) {
    unsigned char next = address + 1;
    unsigned char a0 = read_byte_cpu_bus(bus, address);
    unsigned char a1 = read_byte_cpu_bus(bus, next);
    return a0 | (a1 << 8);
}

void write_byte_cpu_bus(cpu_bus_t *bus,
                        address_t address,
                        unsigned char value) {
    *get_memory_cpu_bus(bus, address) = value;
}

void write_short_cpu_bus(cpu_bus_t *bus,
                         address_t address,
                         unsigned short value) {
    write_byte_cpu_bus(bus, address, value);
    write_byte_cpu_bus(bus, address + 1, value >> 8);
}
