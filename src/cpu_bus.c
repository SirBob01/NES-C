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

address_t mirror_address_cpu_bus(address_t address,
                                 unsigned long prg_ram_size) {
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

unsigned char *get_memory_cpu_bus(cpu_bus_t *bus, address_t address) {
    address_t norm_address =
        mirror_address_cpu_bus(address, bus->rom->header.prg_ram_size);
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

unsigned char read_cpu_bus(cpu_bus_t *bus, address_t address) {
    if (address >= CPU_MAP_ROM) {
        return read_cpu_mapper(bus->mapper, address);
    } else {
        return *get_memory_cpu_bus(bus, address);
    }
}

void write_cpu_bus(cpu_bus_t *bus, address_t address, unsigned char value) {
    if (address >= CPU_MAP_ROM) {
        write_cpu_mapper(bus->mapper, address, value);
    } else {
        *get_memory_cpu_bus(bus, address) = value;
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
