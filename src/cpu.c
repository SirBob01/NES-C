#include "./cpu.h"

cpu_t create_cpu() {
    cpu_t cpu;
    cpu.registers.a = 0;
    cpu.registers.x = 0;
    cpu.registers.y = 0;
    cpu.registers.pc = 0;
    cpu.registers.s = 0xfd;
    cpu.registers.p = 0x34;
    cpu.cycles = 0;
    cpu.memory = allocate_memory(CPU_RAM_SIZE);
    return cpu;
}

void destroy_cpu(cpu_t *cpu) { free_memory(&cpu->memory); }

address_t mirror_address_cpu(address_t address) {
    if (address < CPU_MEMORY_MAP[CPU_MAP_PPU_REG]) {
        // Mirrored RAM region
        address_t base_address = CPU_MEMORY_MAP[CPU_MAP_RAM];
        return base_address + ((address - base_address) % 0x800);
    } else if (address < CPU_MEMORY_MAP[CPU_MAP_APU_IO]) {
        // Mirrored PPU register memory
        address_t base_address = CPU_MEMORY_MAP[CPU_MAP_PPU_REG];
        return base_address + ((address - base_address) % 0x8);
    }
    return address;
}

unsigned char *apply_memory_mapper(cpu_t *cpu, rom_t *rom, address_t address) {
    switch (rom->header.mapper) {
    case 0:
        return apply_mapper0(cpu, rom, address);
    default:
        break;
    }
    return NULL;
}

unsigned char *get_memory_cpu(cpu_t *cpu, rom_t *rom, address_t address) {
    if (address < CPU_MEMORY_MAP[CPU_MAP_CARTRIDGE]) {
        return cpu->memory.buffer + mirror_address_cpu(address);
    } else {
        return apply_memory_mapper(cpu, rom, address);
    }
    return NULL;
}

unsigned char *apply_mapper0(cpu_t *cpu, rom_t *rom, address_t address) {
    bool nrom128 = rom->header.prg_rom_size == 0x4000;
    address_t rom_offset = 0x8000;
    if (address >= rom_offset) {
        address_t rom_addr = address - rom_offset;
        return get_prg_rom(rom) + (rom_addr % (nrom128 * 0x4000));
    } else {
        return cpu->memory.buffer + address;
    }
}
