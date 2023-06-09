#include "./cpu.h"
#include "./ops.h"

void create_cpu(cpu_t *cpu, cpu_bus_t *bus, interrupt_t *interrupt) {
    // Set registers
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->pc = 0;
    cpu->s = 0xfd;

    // Set status flags
    cpu->status.c = false;
    cpu->status.z = false;
    cpu->status.i = true;
    cpu->status.d = false;
    cpu->status.b = false;
    cpu->status.o = false;
    cpu->status.n = false;

    // Cycles on reset
    cpu->cycles = 7;

    // Peripherals
    cpu->bus = bus;
    cpu->interrupt = interrupt;
    cpu->interrupt_vector = CPU_VEC_IRQ_BRK;
}

void destroy_cpu(cpu_t *cpu) {}

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

void push_byte_cpu(cpu_t *cpu, unsigned char value) {
    write_byte_cpu_bus(cpu->bus, 0x100 | cpu->s, value);
    cpu->s--;
}

void push_short_cpu(cpu_t *cpu, unsigned short value) {
    push_byte_cpu(cpu, value >> 8);
    push_byte_cpu(cpu, value);
}

unsigned char pop_byte_cpu(cpu_t *cpu) {
    cpu->s++;
    return read_byte_cpu_bus(cpu->bus, 0x100 | cpu->s);
}

unsigned short pop_short_cpu(cpu_t *cpu) {
    unsigned char a0 = pop_byte_cpu(cpu);
    unsigned char a1 = pop_byte_cpu(cpu);
    return a0 | (a1 << 8);
}

void read_state_cpu(cpu_t *cpu, char *buffer, unsigned buffer_size) {
    unsigned char opcode_byte = read_byte_cpu_bus(cpu->bus, cpu->pc);
    opcode_t opcode = OP_TABLE[opcode_byte];
    unsigned char term_count = ADDRESS_MODE_SIZES[opcode.address_mode];
    switch (term_count) {
    case 1:
        snprintf(buffer,
                 buffer_size,
                 "%04X  %02X        A:%02X X:%02X Y:%02X P:%02X SP:%02X "
                 "PPU:%3d,%3d CYC:"
                 "%lu",
                 cpu->pc,
                 opcode_byte,
                 cpu->a,
                 cpu->x,
                 cpu->y,
                 get_status_cpu(cpu),
                 cpu->s,
                 cpu->bus->ppu->scanline,
                 cpu->bus->ppu->dot,
                 cpu->cycles);
        break;
    case 2:
        snprintf(buffer,
                 buffer_size,
                 "%04X  %02X %02X     A:%02X X:%02X Y:%02X P:%02X SP:%02X "
                 "PPU:%3d,%3d CYC:"
                 "%lu",
                 cpu->pc,
                 opcode_byte,
                 read_byte_cpu_bus(cpu->bus, cpu->pc + 1),
                 cpu->a,
                 cpu->x,
                 cpu->y,
                 get_status_cpu(cpu),
                 cpu->s,
                 cpu->bus->ppu->scanline,
                 cpu->bus->ppu->dot,
                 cpu->cycles);
        break;
    case 3:
        snprintf(buffer,
                 buffer_size,
                 "%04X  %02X %02X %02X  A:%02X X:%02X Y:%02X P:%02X SP:%02X "
                 "PPU:%3d,%3d CYC:"
                 "%lu",
                 cpu->pc,
                 opcode_byte,
                 read_byte_cpu_bus(cpu->bus, cpu->pc + 1),
                 read_byte_cpu_bus(cpu->bus, cpu->pc + 2),
                 cpu->a,
                 cpu->x,
                 cpu->y,
                 get_status_cpu(cpu),
                 cpu->s,
                 cpu->bus->ppu->scanline,
                 cpu->bus->ppu->dot,
                 cpu->cycles);
        break;
    default:
        break;
    }
}

unsigned char fetch_op_cpu(cpu_t *cpu) {
    // Handle NMI, IRQ, and RESET interrupts
    if (cpu->interrupt->nmi) {
        cpu->interrupt_vector = CPU_VEC_NMI;
        return 0;
    }
    if (cpu->interrupt->irq && !cpu->status.i) {
        cpu->interrupt_vector = CPU_VEC_IRQ_BRK;
        return 0;
    }
    if (cpu->interrupt->reset && !cpu->status.i) {
        cpu->interrupt_vector = CPU_VEC_RESET;
        return 0;
    }

    // No interrupts, next instruction from program counter
    return read_byte_cpu_bus(cpu->bus, cpu->pc);
}

operand_t decode_op_cpu(cpu_t *cpu, unsigned char opcode_byte) {
    opcode_t opcode = OP_TABLE[opcode_byte];

    switch (opcode.address_mode) {
    case ADDR_IMMEDIATE:
        return immediate_addr(cpu);
    case ADDR_ZERO_PAGE:
        return zero_page_addr(cpu);
    case ADDR_ZERO_PAGE_X:
        return zero_page_x_addr(cpu);
    case ADDR_ZERO_PAGE_Y:
        return zero_page_y_addr(cpu);
    case ADDR_RELATIVE:
        return relative_addr(cpu);
    case ADDR_ABSOLUTE:
        return absolute_addr(cpu);
    case ADDR_ABSOLUTE_X:
        return absolute_x_addr(cpu);
    case ADDR_ABSOLUTE_Y:
        return absolute_y_addr(cpu);
    case ADDR_INDIRECT:
        return indirect_addr(cpu);
    case ADDR_INDIRECT_X:
        return indirect_x_addr(cpu);
    case ADDR_INDIRECT_Y:
        return indirect_y_addr(cpu);
    default:
        return (operand_t){0, 0};
    }
}

bool execute_op_cpu(cpu_t *cpu, unsigned char opcode_byte, operand_t operand) {
    // Update the program counter.
    opcode_t opcode = OP_TABLE[opcode_byte];
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
    case OP_SHX:
    case OP_SHY:
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
        cpu->x = read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x >> 7;
        break;
    case OP_STX:
        write_byte_cpu_bus(cpu->bus, operand.address, cpu->x);
        break;
    case OP_JSR:
        push_short_cpu(cpu, cpu->pc - 1);
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
        cpu->a = read_byte_cpu_bus(cpu->bus, operand.address);
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
        write_byte_cpu_bus(cpu->bus, operand.address, cpu->a);
        break;
    case OP_BIT: {
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
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
        cpu->pc = pop_short_cpu(cpu) + 1;
        break;
    case OP_SEI:
        cpu->status.i = 1;
        break;
    case OP_SED:
        cpu->status.d = 1;
        break;
    case OP_PHP:
        // Pushed copy should have break flag and bit 5 set
        cpu->status.b = true;
        push_byte_cpu(cpu, get_status_cpu(cpu) | 0x20);
        cpu->status.b = false;
        break;
    case OP_PLA:
        cpu->a = pop_byte_cpu(cpu);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_AND:
        cpu->a &= read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_CMP: {
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
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
        push_byte_cpu(cpu, cpu->a);
        break;
    case OP_PLP: {
        unsigned char status = pop_byte_cpu(cpu);
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
        cpu->a |= read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_CLV:
        cpu->status.o = 0;
        break;
    case OP_EOR:
        cpu->a ^= read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_ADC: {
        unsigned char m = read_byte_cpu_bus(cpu->bus, operand.address);
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
        cpu->y = read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y >> 7;
        break;
    case OP_CPY: {
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
        unsigned char sub = cpu->y - val;
        cpu->status.c = cpu->y >= val;
        cpu->status.z = cpu->y == val;
        cpu->status.n = sub >> 7;
        break;
    }
    case OP_CPX: {
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
        unsigned char sub = cpu->x - val;
        cpu->status.c = cpu->x >= val;
        cpu->status.z = cpu->x == val;
        cpu->status.n = sub >> 7;
        break;
    }
    case OP_SBC: {
        unsigned char m = ~read_byte_cpu_bus(cpu->bus, operand.address);
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
        unsigned char status = pop_byte_cpu(cpu);
        cpu->status.c = status & 0x1;
        cpu->status.z = status & 0x2;
        cpu->status.i = status & 0x4;
        cpu->status.d = status & 0x8;
        cpu->status.o = status & 0x40;
        cpu->status.n = status & 0x80;

        // Pop program counter
        cpu->pc = pop_short_cpu(cpu);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            cpu->status.c = val & 0x1;
            val >>= 1;
            cpu->status.z = val == 0;
            cpu->status.n = false;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            cpu->status.c = val & 0x80;
            val <<= 1;
            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x1;
            val >>= 1;
            val |= (old_c << 7);

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x80;
            val <<= 1;
            val |= old_c;

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
            break;
        }
        }
        break;
    case OP_STY:
        write_byte_cpu_bus(cpu->bus, operand.address, cpu->y);
        break;
    case OP_INC: {
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
        val++;
        write_byte_cpu_bus(cpu->bus, operand.address, val);
        cpu->status.z = val == 0;
        cpu->status.n = val >> 7;
        break;
    }
    case OP_DEC: {
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
        val--;
        write_byte_cpu_bus(cpu->bus, operand.address, val);
        cpu->status.z = val == 0;
        cpu->status.n = val >> 7;
        break;
    }
    case OP_BRK:
        // Push PC + 2 (+1 because PC is already incremented)
        push_short_cpu(cpu, cpu->pc + 1);

        // Push status with b set if software interrupt was triggered
        cpu->status.b = !cpu->interrupt->irq && !cpu->interrupt->nmi &&
                        !cpu->interrupt->reset;
        push_byte_cpu(cpu, get_status_cpu(cpu) | 0x20);
        cpu->status.b = false;

        // Update interrupt disable flag
        cpu->status.i = true;

        // Execute interrupt handler
        cpu->pc = read_short_cpu_bus(cpu->bus, cpu->interrupt_vector);
        break;
    case OP_LAX:
        cpu->a = read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->x = cpu->a;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_SAX:
        write_byte_cpu_bus(cpu->bus, operand.address, cpu->a & cpu->x);
        break;
    case OP_DCP: {
        // DEC
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
        val--;
        write_byte_cpu_bus(cpu->bus, operand.address, val);

        // CMP
        unsigned char sub = cpu->a - val;
        cpu->status.c = cpu->a >= val;
        cpu->status.z = cpu->a == val;
        cpu->status.n = sub >> 7;
        break;
    }
    case OP_ISC: {
        // INC
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
        val++;
        write_byte_cpu_bus(cpu->bus, operand.address, val);

        // SBC
        unsigned char m = ~read_byte_cpu_bus(cpu->bus, operand.address);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            cpu->status.c = val & 0x80;
            val <<= 1;
            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
            break;
        }
        }

        // ORA
        cpu->a |= read_byte_cpu_bus(cpu->bus, operand.address);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x80;
            val <<= 1;
            val |= old_c;

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
            break;
        }
        }

        // AND
        cpu->a &= read_byte_cpu_bus(cpu->bus, operand.address);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            cpu->status.c = val & 0x1;
            val >>= 1;
            cpu->status.z = val == 0;
            cpu->status.n = false;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
            break;
        }
        }

        // EOR
        cpu->a ^= read_byte_cpu_bus(cpu->bus, operand.address);
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
            unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
            unsigned char old_c = cpu->status.c;
            cpu->status.c = val & 0x1;
            val >>= 1;
            val |= (old_c << 7);

            cpu->status.z = val == 0;
            cpu->status.n = val >> 7;
            write_byte_cpu_bus(cpu->bus, operand.address, val);
            break;
        }
        }

        // ADC
        unsigned char m = read_byte_cpu_bus(cpu->bus, operand.address);
        unsigned char n = cpu->a;
        unsigned short res = m + n + cpu->status.c;
        cpu->a = res;
        cpu->status.c = res > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
        break;
    case OP_ANC:
        // AND
        cpu->a &= read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;

        // Set C
        cpu->status.c = cpu->a >> 7;
        break;
    case OP_ALR: // (ASR)
        // AND
        cpu->a &= read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;

        // LSR
        cpu->status.c = cpu->a & 0x1;
        cpu->a >>= 1;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = false; // bit 7 is always 0
        break;
    case OP_ARR: {
        cpu->a &= read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->a >>= 1;
        cpu->a |= (cpu->status.c << 7);

        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;

        cpu->status.c = (cpu->a >> 6) & 0x1;
        cpu->status.o = ((cpu->a >> 5) ^ (cpu->a >> 6)) & 0x1;
        break;
    }
    case OP_SBX: { // (AXS)
        // CMP and DEX at the same time but set flags like CMP
        unsigned char val = read_byte_cpu_bus(cpu->bus, operand.address);
        unsigned char and_res = cpu->a & cpu->x;
        cpu->x = and_res - val;
        cpu->status.c = and_res >= val;
        cpu->status.z = and_res == val;
        cpu->status.n = cpu->x >> 7;
        break;
    }
    case OP_LXA:
        cpu->a = read_byte_cpu_bus(cpu->bus, operand.address);
        cpu->x = cpu->a;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a >> 7;
        break;
    case OP_SHY: {
        address_t target = operand.address;
        unsigned char val = cpu->y & ((target >> 8) + 1);
        if (operand.page_crossed) {
            target = (target & 0xff) | (val << 8);
        }
        write_byte_cpu_bus(cpu->bus, target, val);
        break;
    }
    case OP_SHX: {
        address_t target = operand.address;
        unsigned char val = cpu->x & ((target >> 8) + 1);
        if (operand.page_crossed) {
            target = (target & 0xff) | (val << 8);
        }
        write_byte_cpu_bus(cpu->bus, target, val);
        break;
    }
    case OP_CLI:
        cpu->status.i = false;
        break;
    case OP_NOP:
        break;
    case OP_JAM:
        fprintf(stderr, "JAM opcode encountered: 0x%02X\n", opcode_byte);
        return false;
        break;
    default:
        fprintf(stderr, "Unknown opcode: 0x%02X\n", opcode_byte);
        return false;
        break;
    }

    // Update the cycles
    cpu->cycles += (opcode.cycles + delay_cycles);

    // Reset interrupt vector
    cpu->interrupt_vector = CPU_VEC_IRQ_BRK;

    return true;
}

bool update_cpu(cpu_t *cpu) {
    // Fetch
    unsigned char opcode_byte = fetch_op_cpu(cpu);

    // Decode
    operand_t operands = decode_op_cpu(cpu, opcode_byte);

    // Execute
    return execute_op_cpu(cpu, opcode_byte, operands);
}
