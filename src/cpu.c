#include "./cpu.h"

cpu_t create_cpu() {
    cpu_t cpu;
    cpu.registers.a = 0;
    cpu.registers.x = 0;
    cpu.registers.y = 0;
    cpu.registers.pc = 0;
    cpu.registers.s = 0;
    cpu.registers.p = 0;
    cpu.cycles = 0;
    cpu.memory = allocate_memory(CPU_RAM_SIZE);
    return cpu;
}

unsigned short map_address_cpu(rom_t *rom, unsigned short address) {
    if (address < CPU_MEMORY_MAP[CPU_MAP_PPU_REG]) {
        // Mirrored RAM region
        unsigned base_address = CPU_MEMORY_MAP[CPU_MAP_RAM];
        return base_address + ((address - base_address) % 0x800);
    } else if (address < CPU_MEMORY_MAP[CPU_MAP_APU_IO]) {
        // Mirrored PPU register memory
        unsigned base_address = CPU_MEMORY_MAP[CPU_MAP_PPU_REG];
        return base_address + ((address - base_address) % 0x8);
    } else if (address >= CPU_MEMORY_MAP[CPU_MAP_CARTRIDGE]) {
        // TODO: Bank switching depending on mapping for cartridge section
    }

    // No special transformations needed
    return address;
}

unsigned char read_cpu(cpu_t *cpu, rom_t *rom, unsigned short address) {
    unsigned short ta = map_address_cpu(rom, address);
    return cpu->memory.buffer[ta];
}

void write_cpu(cpu_t *cpu,
               rom_t *rom,
               unsigned short address,
               unsigned char data) {
    unsigned short ta = map_address_cpu(rom, address);
    cpu->memory.buffer[ta] = data;
}

void destroy_cpu(cpu_t *cpu) { free_memory(&cpu->memory); }