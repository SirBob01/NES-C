#include "./cpu.h"
#include "./mapper.h"

cpu_t create_cpu() {
    cpu_t cpu;

    // Set registers
    cpu.a = 0;
    cpu.x = 0;
    cpu.y = 0;
    cpu.pc = 0;
    cpu.s = 0xfd;

    // Set status flags
    cpu.status.c = false;
    cpu.status.z = false;
    cpu.status.i = false;
    cpu.status.d = false;
    cpu.status.b = false;
    cpu.status.o = false;
    cpu.status.n = false;

    cpu.cycles = 0;
    cpu.memory = allocate_memory(CPU_RAM_SIZE);
    return cpu;
}

void destroy_cpu(cpu_t *cpu) { free_memory(&cpu->memory); }

unsigned char get_status_cpu(cpu_t *cpu) {
    unsigned char status = 0;
    status |= cpu->status.c << 0;
    status |= cpu->status.z << 1;
    status |= cpu->status.i << 2;
    status |= cpu->status.d << 3;
    status |= cpu->status.b << 4;
    status |= 1 << 5;
    status |= cpu->status.o << 6;
    status |= cpu->status.n << 7;
    return status;
}

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
        return nrom_mapper(cpu, rom, address);
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

unsigned char read_byte_cpu(cpu_t *cpu, rom_t *rom, address_t address) {
    unsigned char *memory = get_memory_cpu(cpu, rom, address);
    return memory[0];
}

unsigned short read_short_cpu(cpu_t *cpu, rom_t *rom, address_t address) {
    unsigned char *a0 = get_memory_cpu(cpu, rom, address);
    unsigned char *a1 = get_memory_cpu(cpu, rom, address + 1);
    return *a0 | (*a1 << 8);
}

void write_byte_cpu(cpu_t *cpu,
                    rom_t *rom,
                    address_t address,
                    unsigned char value) {
    unsigned char *memory = get_memory_cpu(cpu, rom, address);
    memory[0] = value;
}

void write_short_cpu(cpu_t *cpu,
                     rom_t *rom,
                     address_t address,
                     unsigned short value) {
    unsigned char *a0 = get_memory_cpu(cpu, rom, address);
    unsigned char *a1 = get_memory_cpu(cpu, rom, address + 1);
    *a0 = value;
    *a1 = value >> 8;
}

void push_byte_cpu(cpu_t *cpu, rom_t *rom, unsigned char value) {
    cpu->s--;
    write_byte_cpu(cpu, rom, 0x100 | cpu->s, value);
}

void push_short_cpu(cpu_t *cpu, rom_t *rom, unsigned short value) {
    cpu->s -= 2;
    write_short_cpu(cpu, rom, 0x100 | cpu->s, value);
}

void pop_byte_cpu(cpu_t *cpu, rom_t *rom) { cpu->s++; }

void pop_short_cpu(cpu_t *cpu, rom_t *rom) { cpu->s += 2; }

unsigned char peek_byte_cpu(cpu_t *cpu, rom_t *rom) {
    return read_byte_cpu(cpu, rom, 0x100 | cpu->s);
}

unsigned short peek_short_cpu(cpu_t *cpu, rom_t *rom) {
    return read_short_cpu(cpu, rom, 0x100 | cpu->s);
}

bool update_cpu(cpu_t *cpu, rom_t *rom) {
    unsigned char opcode = read_byte_cpu(cpu, rom, cpu->pc);
    switch (opcode) {
    default:
        break;
    }
    return true;
}
