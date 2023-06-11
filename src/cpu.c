#include "./cpu.h"
#include "./mapper.h"
#include "./ops.h"

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
    cpu.status.i = true;
    cpu.status.d = false;
    cpu.status.b = false;
    cpu.status.o = false;
    cpu.status.n = false;

    // Cycles on reset
    cpu.cycles = 7;

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
    return *memory;
}

unsigned short read_short_cpu(cpu_t *cpu, rom_t *rom, address_t address) {
    unsigned char *a0 = get_memory_cpu(cpu, rom, address);
    unsigned char *a1 = get_memory_cpu(cpu, rom, address + 1);
    return *a0 | (*a1 << 8);
}

unsigned short
read_short_zp_cpu(cpu_t *cpu, rom_t *rom, unsigned char address) {
    unsigned char next = address + 1;
    unsigned char *a0 = get_memory_cpu(cpu, rom, address);
    unsigned char *a1 = get_memory_cpu(cpu, rom, next);
    return *a0 | (*a1 << 8);
}

void write_byte_cpu(cpu_t *cpu,
                    rom_t *rom,
                    address_t address,
                    unsigned char value) {
    unsigned char *memory = get_memory_cpu(cpu, rom, address);
    *memory = value;
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
    write_byte_cpu(cpu, rom, 0x100 | cpu->s, value);
    cpu->s--;
}

void push_short_cpu(cpu_t *cpu, rom_t *rom, unsigned short value) {
    push_byte_cpu(cpu, rom, value >> 8);
    push_byte_cpu(cpu, rom, value);
}

unsigned char pop_byte_cpu(cpu_t *cpu, rom_t *rom) {
    cpu->s++;
    return read_byte_cpu(cpu, rom, 0x100 | cpu->s);
}

unsigned short pop_short_cpu(cpu_t *cpu, rom_t *rom) {
    unsigned char a0 = pop_byte_cpu(cpu, rom);
    unsigned char a1 = pop_byte_cpu(cpu, rom);
    return a0 | (a1 << 8);
}

void print_cpu_state(cpu_t *cpu, rom_t *rom, unsigned char opcode_byte) {
    opcode_t opcode = OP_TABLE[opcode_byte];
    unsigned char term_count = ADDRESS_MODE_SIZES[opcode.address_mode];
    switch (term_count) {
    case 1:
        printf("%04X  %02X        A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:"
               "%lu\n",
               cpu->pc,
               opcode_byte,
               cpu->a,
               cpu->x,
               cpu->y,
               get_status_cpu(cpu),
               cpu->s,
               cpu->cycles);
        break;
    case 2:
        printf("%04X  %02X %02X     A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:"
               "%lu\n",
               cpu->pc,
               opcode_byte,
               read_byte_cpu(cpu, rom, cpu->pc + 1),
               cpu->a,
               cpu->x,
               cpu->y,
               get_status_cpu(cpu),
               cpu->s,
               cpu->cycles);
        break;
    case 3:
        printf("%04X  %02X %02X %02X  A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:"
               "%lu\n",
               cpu->pc,
               opcode_byte,
               read_byte_cpu(cpu, rom, cpu->pc + 1),
               read_byte_cpu(cpu, rom, cpu->pc + 2),
               cpu->a,
               cpu->x,
               cpu->y,
               get_status_cpu(cpu),
               cpu->s,
               cpu->cycles);
        break;
    default:
        break;
    }
}

bool update_cpu(cpu_t *cpu, rom_t *rom) {
    // Fetch
    unsigned char opcode_byte = read_byte_cpu(cpu, rom, cpu->pc);
    opcode_t opcode = OP_TABLE[opcode_byte];
    operand_t operand = {0, 0};

    // Decode
    switch (opcode.address_mode) {
    case ADDR_IMMEDIATE:
        operand = addr_immediate(cpu, rom);
        break;
    case ADDR_ZERO_PAGE:
        operand = addr_zero_page(cpu, rom);
        break;
    case ADDR_ZERO_PAGE_X:
        operand = addr_zero_page_x(cpu, rom);
        break;
    case ADDR_ZERO_PAGE_Y:
        operand = addr_zero_page_y(cpu, rom);
        break;
    case ADDR_RELATIVE:
        operand = addr_relative(cpu, rom);
        break;
    case ADDR_ABSOLUTE:
        operand = addr_absolute(cpu, rom);
        break;
    case ADDR_ABSOLUTE_X:
        operand = addr_absolute_x(cpu, rom);
        break;
    case ADDR_ABSOLUTE_Y:
        operand = addr_absolute_y(cpu, rom);
        break;
    case ADDR_INDIRECT:
        operand = addr_indirect(cpu, rom);
        break;
    case ADDR_INDIRECT_X:
        operand = addr_indirect_x(cpu, rom);
        break;
    case ADDR_INDIRECT_Y:
        operand = addr_indirect_y(cpu, rom);
        break;
    default:
        break;
    }

#ifndef NDEBUG
    print_cpu_state(cpu, rom, opcode_byte);
#endif

    // Update the program counter.
    cpu->pc += ADDRESS_MODE_SIZES[opcode.address_mode];

    // Set number of delay cycles
    unsigned char delay_cycles;
    switch (opcode.op_mode) {
    case OP_STA:
    case OP_STX:
    case OP_STY:
    case OP_SAX:
    case OP_DCP:
    case OP_ISC:
    case OP_SLO:
    case OP_RLA:
    case OP_SRE:
    case OP_RRA:
        delay_cycles = 0;
        break;
    default:
        delay_cycles = operand.page_crossed;
        break;
    }

    // Execute
    switch (opcode.op_mode) {
    case OP_JMP:
        cpu->pc = operand.address;
        break;
    case OP_LDX:
        cpu->x = read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x >> 7;
        break;
    case OP_STX:
        write_byte_cpu(cpu, rom, operand.address, cpu->x);
        break;
    case OP_JSR:
        push_short_cpu(cpu, rom, cpu->pc - 1);
        cpu->pc = operand.address;
        break;
    case OP_SEC:
        cpu->status.c = 1;
        break;
    case OP_BCS:
        if (cpu->status.c) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_CLC:
        cpu->status.c = 0;
        break;
    case OP_BCC:
        if (!cpu->status.c) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_LDA:
        cpu->a = read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_BEQ:
        if (cpu->status.z) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_BNE:
        if (!cpu->status.z) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_STA:
        write_byte_cpu(cpu, rom, operand.address, cpu->a);
        break;
    case OP_BIT: {
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = (cpu->a & val) == 0;
        cpu->status.n = val >> 7;
        cpu->status.o = (val >> 6) & 1;
        break;
    }
    case OP_BVS:
        if (cpu->status.o) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_BVC:
        if (!cpu->status.o) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_BPL:
        if (!cpu->status.n) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_RTS:
        cpu->pc = pop_short_cpu(cpu, rom) + 1;
        break;
    case OP_SEI:
        cpu->status.i = 1;
        break;
    case OP_SED:
        cpu->status.d = 1;
        break;
    case OP_PHP:
        // Pushed copy should have break flag set (0x10)
        push_byte_cpu(cpu, rom, get_status_cpu(cpu) | 0x10);
        break;
    case OP_PLA:
        cpu->a = pop_byte_cpu(cpu, rom);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_AND:
        cpu->a &= read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_CMP: {
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        unsigned char sub = cpu->a - val;
        cpu->status.c = cpu->a >= val;
        cpu->status.z = cpu->a == val;
        cpu->status.n = sub >> 7;
        break;
    }
    case OP_CLD:
        cpu->status.d = 0;
        break;
    case OP_PHA:
        push_byte_cpu(cpu, rom, cpu->a);
        break;
    case OP_PLP: {
        unsigned char status = pop_byte_cpu(cpu, rom);
        cpu->status.c = status & 0x1;
        cpu->status.z = status & 0x2;
        cpu->status.i = status & 0x4;
        cpu->status.d = status & 0x8;
        cpu->status.o = status & 0x40;
        cpu->status.n = status & 0x80;
        break;
    }
    case OP_BMI:
        if (cpu->status.n) {
            cpu->pc = operand.address;
            delay_cycles++;
        } else {
            delay_cycles = 0;
        }
        break;
    case OP_ORA:
        cpu->a |= read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_CLV:
        cpu->status.o = 0;
        break;
    case OP_EOR:
        cpu->a ^= read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_ADC: {
        unsigned char m = read_byte_cpu(cpu, rom, operand.address);
        unsigned char n = cpu->a;
        unsigned short res = m + n + cpu->status.c;
        cpu->a = res;
        cpu->status.c = res > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
        break;
    }
    case OP_LDY:
        cpu->y = read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y >> 7;
        break;
    case OP_CPY: {
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        unsigned char sub = cpu->y - val;
        cpu->status.c = cpu->y >= val;
        cpu->status.z = cpu->y == val;
        cpu->status.n = sub >> 7;
        break;
    }
    case OP_CPX: {
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        unsigned char sub = cpu->x - val;
        cpu->status.c = cpu->x >= val;
        cpu->status.z = cpu->x == val;
        cpu->status.n = sub >> 7;
        break;
    }
    case OP_SBC: {
        unsigned char m = ~read_byte_cpu(cpu, rom, operand.address);
        unsigned char n = cpu->a;
        unsigned short res = m + n + cpu->status.c;
        cpu->a = res;
        cpu->status.c = res > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
        break;
    }
    case OP_INY:
        cpu->y++;
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y >> 7;
        break;
    case OP_INX:
        cpu->x++;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x >> 7;
        break;
    case OP_DEY:
        cpu->y--;
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y >> 7;
        break;
    case OP_DEX:
        cpu->x--;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x >> 7;
        break;
    case OP_TAY:
        cpu->y = cpu->a;
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y >> 7;
        break;
    case OP_TAX:
        cpu->x = cpu->a;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x >> 7;
        break;
    case OP_TYA:
        cpu->a = cpu->y;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_TXA:
        cpu->a = cpu->x;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_TSX:
        cpu->x = cpu->s;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x >> 7;
        break;
    case OP_TXS:
        cpu->s = cpu->x;
        break;
    case OP_RTI: {
        // Pop status register
        unsigned char status = pop_byte_cpu(cpu, rom);
        cpu->status.c = status & 0x1;
        cpu->status.z = status & 0x2;
        cpu->status.i = status & 0x4;
        cpu->status.d = status & 0x8;
        cpu->status.b = status & 0x10;
        cpu->status.o = status & 0x40;
        cpu->status.n = status & 0x80;

        // Pop program counter
        cpu->pc = pop_short_cpu(cpu, rom);
        break;
    }
    case OP_LSR:
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR:
            cpu->status.c = cpu->a & 0x1;
            cpu->a >>= 1;
            cpu->status.z = cpu->a == 0;
            cpu->status.n = false; // bit 7 is always 0
            break;
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            cpu->status.c = val & 0x1;
            val >>= 1;
            cpu->status.z = val == 0;
            cpu->status.n = false;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }
        break;
    case OP_ASL:
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR:
            cpu->status.c = cpu->a & 0x80;
            cpu->a <<= 1;
            cpu->status.z = cpu->a == 0;
            cpu->status.n = cpu->a >> 7;
            break;
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            cpu->status.c = val & 0x80;
            val <<= 1;
            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }
        break;
    case OP_ROR:
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR: {
            unsigned char old_c = cpu->status.c;
            cpu->status.c = cpu->a & 0x1;
            cpu->a >>= 1;
            cpu->a |= (old_c << 7);

            cpu->status.z = cpu->a == 0;
            cpu->status.n = cpu->a >> 7;
            break;
        }
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x1;
            val >>= 1;
            val |= (old_c << 7);

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }
        break;
    case OP_ROL:
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR: {
            unsigned char old_c = cpu->status.c;
            cpu->status.c = cpu->a & 0x80;
            cpu->a <<= 1;
            cpu->a |= old_c;

            cpu->status.z = cpu->a == 0;
            cpu->status.n = cpu->a >> 7;
            break;
        }
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x80;
            val <<= 1;
            val |= old_c;

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }
        break;
    case OP_STY:
        write_byte_cpu(cpu, rom, operand.address, cpu->y);
        break;
    case OP_INC: {
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        val++;
        write_byte_cpu(cpu, rom, operand.address, val);
        cpu->status.z = val == 0;
        cpu->status.n = val >> 7;
        break;
    }
    case OP_DEC: {
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        val--;
        write_byte_cpu(cpu, rom, operand.address, val);
        cpu->status.z = val == 0;
        cpu->status.n = val >> 7;
        break;
    }
    case OP_BRK:
        push_short_cpu(cpu, rom, cpu->pc + 2);
        push_byte_cpu(cpu, rom, get_status_cpu(cpu) | 0x10);
        cpu->pc = read_short_cpu(cpu, rom, CPU_VEC_IRQ_BRK);
        cpu->status.b = true;
        break;
    case OP_LAX:
        cpu->a = read_byte_cpu(cpu, rom, operand.address);
        cpu->x = cpu->a;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_SAX:
        write_byte_cpu(cpu, rom, operand.address, cpu->a & cpu->x);
        break;
    case OP_DCP: {
        // DEC
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        val--;
        write_byte_cpu(cpu, rom, operand.address, val);

        // CMP
        unsigned char sub = cpu->a - val;
        cpu->status.c = cpu->a >= val;
        cpu->status.z = cpu->a == val;
        cpu->status.n = sub >> 7;
        break;
    }
    case OP_ISC: {
        // INC
        unsigned char val = read_byte_cpu(cpu, rom, operand.address);
        val++;
        write_byte_cpu(cpu, rom, operand.address, val);

        // SBC
        unsigned char m = ~read_byte_cpu(cpu, rom, operand.address);
        unsigned char n = cpu->a;
        unsigned short res = m + n + cpu->status.c;
        cpu->a = res;
        cpu->status.c = res > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
        break;
    }
    case OP_SLO:
        // ASL
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR:
            cpu->status.c = cpu->a & 0x80;
            cpu->a <<= 1;
            cpu->status.z = cpu->a == 0;
            cpu->status.n = cpu->a >> 7;
            break;
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            cpu->status.c = val & 0x80;
            val <<= 1;
            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }

        // ORA
        cpu->a |= read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_RLA:
        // ROL
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR: {
            unsigned char old_c = cpu->status.c;
            cpu->status.c = cpu->a & 0x80;
            cpu->a <<= 1;
            cpu->a |= old_c;

            cpu->status.z = cpu->a == 0;
            cpu->status.n = cpu->a >> 7;
            break;
        }
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x80;
            val <<= 1;
            val |= old_c;

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }

        // AND
        cpu->a &= read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_SRE:
        // LSR
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR:
            cpu->status.c = cpu->a & 0x1;
            cpu->a >>= 1;
            cpu->status.z = cpu->a == 0;
            cpu->status.n = false; // bit 7 is always 0
            break;
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            cpu->status.c = val & 0x1;
            val >>= 1;
            cpu->status.z = val == 0;
            cpu->status.n = false;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }

        // EOR
        cpu->a ^= read_byte_cpu(cpu, rom, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_RRA:
        // ROR
        switch (opcode.address_mode) {
        case ADDR_ACCUMULATOR: {
            unsigned char old_c = cpu->status.c;
            cpu->status.c = cpu->a & 0x1;
            cpu->a >>= 1;
            cpu->a |= (old_c << 7);

            cpu->status.z = cpu->a == 0;
            cpu->status.n = cpu->a >> 7;
            break;
        }
        default: {
            unsigned char val = read_byte_cpu(cpu, rom, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x1;
            val >>= 1;
            val |= (old_c << 7);

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu(cpu, rom, operand.address, val);
            break;
        }
        }

        // ADC
        unsigned char m = read_byte_cpu(cpu, rom, operand.address);
        unsigned char n = cpu->a;
        unsigned short res = m + n + cpu->status.c;
        cpu->a = res;
        cpu->status.c = res > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
        break;
    case OP_NOP:
        break;
    case OP_JAM:
        fprintf(stderr, "JAM opcode encountered: 0x%02X\n", opcode_byte);
        exit(1);
        break;
    default:
        fprintf(stderr, "Unknown opcode: 0x%02X\n", opcode_byte);
        exit(1);
        break;
    }

    // Update the cycles
    cpu->cycles += (opcode.cycles + delay_cycles);
    return true;
}
