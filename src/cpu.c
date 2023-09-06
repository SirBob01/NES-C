#include "./cpu.h"
#include "./ops.h"
#include "cpu_bus.h"

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

void push_stack_cpu(cpu_t *cpu, unsigned char value) {
    write_cpu_bus(cpu->bus, 0x100 | cpu->s, value);
    cpu->s--;
}

unsigned char pop_stack_cpu(cpu_t *cpu) {
    cpu->s++;
    return read_cpu_bus(cpu->bus, 0x100 | cpu->s);
}

void read_state_cpu(cpu_t *cpu, char *buffer, unsigned buffer_size) {
    unsigned char opcode = read_cpu_bus(cpu->bus, cpu->pc);
    operation_t operation = OP_TABLE[opcode];
    unsigned char term_count = ADDRESS_MODE_SIZES[operation.address_mode];
    switch (term_count) {
    case 1:
        snprintf(buffer,
                 buffer_size,
                 "%04X  %02X        A:%02X X:%02X Y:%02X P:%02X SP:%02X "
                 "PPU:%3d,%3d CYC:"
                 "%lu",
                 cpu->pc,
                 opcode,
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
                 opcode,
                 read_cpu_bus(cpu->bus, cpu->pc + 1),
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
                 opcode,
                 read_cpu_bus(cpu->bus, cpu->pc + 1),
                 read_cpu_bus(cpu->bus, cpu->pc + 2),
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

void update_peripherals_cpu(cpu_t *cpu) {
    while (cpu->bus->ppu->cycles < cpu->cycles * 3) {
        update_ppu(cpu->bus->ppu);
    }
    while (cpu->bus->apu->cycles * 2 < cpu->cycles) {
        update_apu(cpu->bus->apu);
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
    cpu->cycles++;
    unsigned char opcode = read_cpu_bus(cpu->bus, cpu->pc++);
    update_peripherals_cpu(cpu);
    return opcode;
}

address_t decode_op_cpu(cpu_t *cpu, unsigned char opcode) {
    operation_t operation = OP_TABLE[opcode];

    // Skip decoding for special opcodes
    switch (operation.mnemonic) {
    case OP_JSR:
    case OP_BRK:
    case OP_RTI:
    case OP_RTS:
    case OP_PHA:
    case OP_PHP:
    case OP_PLA:
    case OP_PLP:
        return 0;
    default:
        break;
    }

    // Decode operand based on addressing mode
    switch (operation.address_mode) {
    case ADDR_ABSOLUTE: {
        cpu->cycles++;
        unsigned char adl = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char adh = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);
        return (adh << 8) | adl;
    }
    case ADDR_IMMEDIATE:
        return cpu->pc++;
    case ADDR_ZERO_PAGE: {
        cpu->cycles++;
        unsigned char address = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);
        return address;
    }
    case ADDR_IMPLIED:
    case ADDR_ACCUMULATOR:
        // Skip decoding for implied and accumulator addressing modes
        return 0;
    case ADDR_RELATIVE:
        cpu->cycles++;
        signed char offset = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);
        return offset + cpu->pc;
    case ADDR_INDIRECT_X:
        cpu->cycles++;
        unsigned char base = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->cycles++;
        unsigned char adl_address = base + cpu->x;
        unsigned char adl = read_cpu_bus(cpu->bus, adl_address);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char adh_address = adl_address + 1;
        unsigned char adh = read_cpu_bus(cpu->bus, adh_address);
        update_peripherals_cpu(cpu);

        return (adh << 8) | adl;
    case ADDR_INDIRECT_Y: {
        cpu->cycles++;
        unsigned char ptr_address = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char adl_address = ptr_address;
        unsigned char adl = read_cpu_bus(cpu->bus, adl_address);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char adh_address = ptr_address + 1;
        unsigned char adh = read_cpu_bus(cpu->bus, adh_address);
        update_peripherals_cpu(cpu);

        // Check for page crossing (or write operation)
        address_t base_address = ((adh << 8) | adl);
        address_t address = base_address + cpu->y;
        if ((base_address & 0xff00) != (address & 0xff00) ||
            operation.group == OPGROUP_W) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, address);
            update_peripherals_cpu(cpu);
        }
        return address;
    }
    case ADDR_INDIRECT: {
        cpu->cycles++;
        unsigned char ptr_adl = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char ptr_adh = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        address_t adl_address = (ptr_adh << 8) | ptr_adl;
        unsigned char adl = read_cpu_bus(cpu->bus, adl_address);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        address_t adh_address = (ptr_adh << 8) | ((ptr_adl + 1) & 0xff);
        unsigned char adh = read_cpu_bus(cpu->bus, adh_address);
        update_peripherals_cpu(cpu);

        return (adh << 8) | adl;
    }
    case ADDR_ABSOLUTE_Y: {
        cpu->cycles++;
        unsigned char adl = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char adh = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        // Check for page crossing (or write operation)
        address_t base_address = ((adh << 8) | adl);
        address_t address = base_address + cpu->y;

        if ((base_address & 0xff00) != (address & 0xff00) ||
            operation.group == OPGROUP_W) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, address);
            update_peripherals_cpu(cpu);
        }
        return address;
    }
    case ADDR_ABSOLUTE_X: {
        cpu->cycles++;
        unsigned char adl = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char adh = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        // Check for page crossing (or write operation)
        address_t base_address = ((adh << 8) | adl);
        address_t address = base_address + cpu->x;

        if ((base_address & 0xff00) != (address & 0xff00) ||
            operation.group == OPGROUP_W || operation.group == OPGROUP_RW) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, address);
            update_peripherals_cpu(cpu);
        }
        return address;
    }
    case ADDR_ZERO_PAGE_X: {
        cpu->cycles++;
        unsigned char base = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char address = base + cpu->x;
        return address;
    }
    case ADDR_ZERO_PAGE_Y: {
        cpu->cycles++;
        unsigned char base = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char address = base + cpu->y;
        return address;
    }
    }
}

bool execute_op_cpu(cpu_t *cpu, unsigned char opcode, address_t operand) {
    operation_t operation = OP_TABLE[opcode];

    switch (operation.mnemonic) {
    case OP_JMP:
        cpu->pc = operand;
        break;
    case OP_LDX:
        cpu->cycles++;
        cpu->x = read_cpu_bus(cpu->bus, operand);
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_STX:
        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, cpu->x);
        update_peripherals_cpu(cpu);
        break;
    case OP_JSR:
        cpu->cycles++;
        unsigned char adl = read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->cycles++;
        push_stack_cpu(cpu, cpu->pc >> 8);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        push_stack_cpu(cpu, cpu->pc & 0xff);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char adh = read_cpu_bus(cpu->bus, cpu->pc++);
        cpu->pc = (adh << 8) | adl;
        update_peripherals_cpu(cpu);
        break;
    case OP_NOP:
        cpu->cycles++;
        update_peripherals_cpu(cpu);
        break;
    case OP_SEC:
        cpu->cycles++;
        cpu->status.c = true;
        update_peripherals_cpu(cpu);
        break;
    case OP_BCS:
        if (cpu->status.c) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_CLC:
        cpu->cycles++;
        cpu->status.c = false;
        update_peripherals_cpu(cpu);
        break;
    case OP_BCC:
        if (!cpu->status.c) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_LDA:
        cpu->cycles++;
        cpu->a = read_cpu_bus(cpu->bus, operand);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_BEQ:
        if (cpu->status.z) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_BNE:
        if (!cpu->status.z) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_STA:
        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, cpu->a);
        update_peripherals_cpu(cpu);
        break;
    case OP_BIT:
        cpu->cycles++;
        unsigned char value = read_cpu_bus(cpu->bus, operand);
        cpu->status.z = (cpu->a & value) == 0;
        cpu->status.n = value & 0x80;
        cpu->status.o = value & 0x40;
        update_peripherals_cpu(cpu);
        break;
    case OP_BVS:
        if (cpu->status.o) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_BVC:
        if (!cpu->status.o) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_BPL:
        if (!cpu->status.n) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_RTS:
        cpu->cycles++;
        read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->cycles++;
        unsigned char pcl = pop_stack_cpu(cpu);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->cycles++;
        unsigned char pch = pop_stack_cpu(cpu);
        cpu->pc = ((pch << 8) | pcl) + 1;
        update_peripherals_cpu(cpu);
        break;
    case OP_SEI:
        cpu->cycles++;
        cpu->status.i = true;
        update_peripherals_cpu(cpu);
        break;
    case OP_SED:
        cpu->cycles++;
        cpu->status.d = true;
        update_peripherals_cpu(cpu);
        break;
    case OP_PHP:
        cpu->cycles++;
        read_cpu_bus(cpu->bus, cpu->pc);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->status.b = true;
        push_stack_cpu(cpu, get_status_cpu(cpu));
        cpu->status.b = false;
        update_peripherals_cpu(cpu);
        break;
    case OP_PLA:
        cpu->cycles++;
        read_cpu_bus(cpu->bus, cpu->pc);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->cycles++;
        cpu->a = pop_stack_cpu(cpu);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_AND:
        cpu->cycles++;
        cpu->a &= read_cpu_bus(cpu->bus, operand);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_CMP:
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        unsigned char sub = cpu->a - val;
        cpu->status.c = cpu->a >= val;
        cpu->status.z = cpu->a == val;
        cpu->status.n = sub & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_CLD:
        cpu->cycles++;
        cpu->status.d = false;
        update_peripherals_cpu(cpu);
        break;
    case OP_PHA:
        cpu->cycles++;
        read_cpu_bus(cpu->bus, cpu->pc);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        push_stack_cpu(cpu, cpu->a);
        update_peripherals_cpu(cpu);
        break;
    case OP_PLP:
        cpu->cycles++;
        read_cpu_bus(cpu->bus, cpu->pc);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->cycles++;
        unsigned char status = pop_stack_cpu(cpu);
        cpu->status.c = status & 0x1;
        cpu->status.z = status & 0x2;
        cpu->status.i = status & 0x4;
        cpu->status.d = status & 0x8;
        cpu->status.o = status & 0x40;
        cpu->status.n = status & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_BMI:
        if (cpu->status.n) {
            cpu->cycles++;
            read_cpu_bus(cpu->bus, cpu->pc);
            update_peripherals_cpu(cpu);

            if ((operand & 0xff00) != (cpu->pc & 0xff00)) {
                cpu->cycles++;
                read_cpu_bus(cpu->bus, cpu->pc);
                update_peripherals_cpu(cpu);
            }
            cpu->pc = operand;
        }
        break;
    case OP_ORA:
        cpu->cycles++;
        cpu->a |= read_cpu_bus(cpu->bus, operand);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_CLV:
        cpu->cycles++;
        cpu->status.o = false;
        update_peripherals_cpu(cpu);
        break;
    case OP_EOR:
        cpu->cycles++;
        cpu->a ^= read_cpu_bus(cpu->bus, operand);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_ADC: {
        cpu->cycles++;
        unsigned char m = read_cpu_bus(cpu->bus, operand);
        unsigned char n = cpu->a;
        unsigned short res = m + n + cpu->status.c;
        cpu->a = res;
        cpu->status.c = res > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
        update_peripherals_cpu(cpu);
    } break;
    case OP_LDY:
        cpu->cycles++;
        cpu->y = read_cpu_bus(cpu->bus, operand);
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_CPY:
        cpu->cycles++;
        unsigned char valy = read_cpu_bus(cpu->bus, operand);
        unsigned char suby = cpu->y - valy;
        cpu->status.c = cpu->y >= valy;
        cpu->status.z = cpu->y == valy;
        cpu->status.n = suby & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_CPX:
        cpu->cycles++;
        unsigned char valx = read_cpu_bus(cpu->bus, operand);
        unsigned char subx = cpu->x - valx;
        cpu->status.c = cpu->x >= valx;
        cpu->status.z = cpu->x == valx;
        cpu->status.n = subx & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_SBC: {
        cpu->cycles++;
        unsigned char m = ~read_cpu_bus(cpu->bus, operand);
        unsigned char n = cpu->a;
        unsigned short res = m + n + cpu->status.c;
        cpu->a = res;
        cpu->status.c = res > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
        update_peripherals_cpu(cpu);
    } break;
    case OP_INY:
        cpu->cycles++;
        cpu->y++;
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_INX:
        cpu->cycles++;
        cpu->x++;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_DEY:
        cpu->cycles++;
        cpu->y--;
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_DEX:
        cpu->cycles++;
        cpu->x--;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_TAY:
        cpu->cycles++;
        cpu->y = cpu->a;
        cpu->status.z = cpu->y == 0;
        cpu->status.n = cpu->y & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_TAX:
        cpu->cycles++;
        cpu->x = cpu->a;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_TYA:
        cpu->cycles++;
        cpu->a = cpu->y;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_TXA:
        cpu->cycles++;
        cpu->a = cpu->x;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_TSX:
        cpu->cycles++;
        cpu->x = cpu->s;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_TXS:
        cpu->cycles++;
        cpu->s = cpu->x;
        update_peripherals_cpu(cpu);
        break;
    case OP_RTI: {
        cpu->cycles++;
        read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->cycles++;
        unsigned char status = pop_stack_cpu(cpu);
        cpu->status.c = status & 0x1;
        cpu->status.z = status & 0x2;
        cpu->status.i = status & 0x4;
        cpu->status.d = status & 0x8;
        cpu->status.o = status & 0x40;
        cpu->status.n = status & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char pcl = pop_stack_cpu(cpu);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char pch = pop_stack_cpu(cpu);
        cpu->pc = (pch << 8) | pcl;
        update_peripherals_cpu(cpu);
    } break;
    case OP_LSR: {
        cpu->cycles++;
        unsigned char val = operation.address_mode == ADDR_ACCUMULATOR
                                ? cpu->a
                                : read_cpu_bus(cpu->bus, operand);
        cpu->status.c = val & 0x1;
        unsigned char result = val >> 1;
        cpu->status.z = result == 0;
        cpu->status.n = false;
        update_peripherals_cpu(cpu);

        if (operation.address_mode != ADDR_ACCUMULATOR) {
            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, val);
            update_peripherals_cpu(cpu);

            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, result);
            update_peripherals_cpu(cpu);
        } else {
            cpu->a = result;
        }
    } break;
    case OP_ASL: {
        cpu->cycles++;
        unsigned char val = operation.address_mode == ADDR_ACCUMULATOR
                                ? cpu->a
                                : read_cpu_bus(cpu->bus, operand);
        cpu->status.c = val & 0x80;
        unsigned char result = val << 1;
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        if (operation.address_mode != ADDR_ACCUMULATOR) {
            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, val);
            update_peripherals_cpu(cpu);

            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, result);
            update_peripherals_cpu(cpu);
        } else {
            cpu->a = result;
        }
    } break;
    case OP_ROR: {
        cpu->cycles++;
        unsigned char val = operation.address_mode == ADDR_ACCUMULATOR
                                ? cpu->a
                                : read_cpu_bus(cpu->bus, operand);
        bool c = cpu->status.c;
        cpu->status.c = val & 0x1;
        unsigned char result = (val >> 1) | (c << 7);
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        if (operation.address_mode != ADDR_ACCUMULATOR) {
            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, val);
            update_peripherals_cpu(cpu);

            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, result);
            update_peripherals_cpu(cpu);
        } else {
            cpu->a = result;
        }
    } break;
    case OP_ROL: {
        cpu->cycles++;
        unsigned char val = operation.address_mode == ADDR_ACCUMULATOR
                                ? cpu->a
                                : read_cpu_bus(cpu->bus, operand);
        bool c = cpu->status.c;
        cpu->status.c = val & 0x80;
        unsigned char result = (val << 1) | c;
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        if (operation.address_mode != ADDR_ACCUMULATOR) {
            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, val);
            update_peripherals_cpu(cpu);

            cpu->cycles++;
            write_cpu_bus(cpu->bus, operand, result);
            update_peripherals_cpu(cpu);
        } else {
            cpu->a = result;
        }
    } break;
    case OP_STY:
        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, cpu->y);
        update_peripherals_cpu(cpu);
        break;
    case OP_INC: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        unsigned char result = val + 1;
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);
    } break;
    case OP_DEC: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        unsigned char result = val - 1;
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);
    } break;
    case OP_LAX:
        cpu->cycles++;
        cpu->a = read_cpu_bus(cpu->bus, operand);
        cpu->x = cpu->a;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
        break;
    case OP_SAX:
        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, cpu->a & cpu->x);
        update_peripherals_cpu(cpu);
        break;
    case OP_DCP: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        unsigned char result = val - 1;
        cpu->status.c = cpu->a >= result;
        cpu->status.z = cpu->a == result;
        cpu->status.n = (cpu->a - result) & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);
    } break;
    case OP_ISC: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        unsigned char result = val + 1;
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);

        unsigned char m = ~result;
        unsigned char n = cpu->a;
        unsigned short sub_result = m + n + cpu->status.c;
        cpu->a = sub_result;
        cpu->status.c = sub_result > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
    } break;
    case OP_SLO: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        cpu->status.c = val & 0x80;
        unsigned char result = val << 1;
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);

        cpu->a |= result;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
    } break;
    case OP_RLA: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        bool c = cpu->status.c;
        cpu->status.c = val & 0x80;
        unsigned char result = (val << 1) | c;
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);

        cpu->a &= result;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
    } break;
    case OP_SRE: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        cpu->status.c = val & 0x1;
        unsigned char result = val >> 1;
        cpu->status.z = result == 0;
        cpu->status.n = false;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);

        cpu->a ^= result;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
    } break;
    case OP_RRA: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        bool c = cpu->status.c;
        cpu->status.c = val & 0x1;
        unsigned char result = (val >> 1) | (c << 7);
        cpu->status.z = result == 0;
        cpu->status.n = result & 0x80;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, val);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        write_cpu_bus(cpu->bus, operand, result);
        update_peripherals_cpu(cpu);

        unsigned char m = result;
        unsigned char n = cpu->a;
        unsigned short sub_result = m + n + cpu->status.c;
        cpu->a = sub_result;
        cpu->status.c = sub_result > 0xff;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        cpu->status.o = ((m ^ cpu->a) & (n ^ cpu->a) & 0x80) > 0;
    } break;
    case OP_ANC: {
        cpu->cycles++;
        cpu->a &= read_cpu_bus(cpu->bus, operand);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        cpu->status.c = cpu->status.n;
        update_peripherals_cpu(cpu);
    } break;
    case OP_ALR: {
        cpu->cycles++;
        cpu->a &= read_cpu_bus(cpu->bus, operand);
        cpu->status.c = cpu->a & 0x1;
        cpu->a >>= 1;
        cpu->status.z = cpu->a == 0;
        cpu->status.n = false;
        update_peripherals_cpu(cpu);
    } break;
    case OP_ARR: {
        cpu->cycles++;
        cpu->a &= read_cpu_bus(cpu->bus, operand);
        cpu->a = (cpu->a >> 1) | (cpu->status.c << 7);
        cpu->status.z = cpu->a == 0;
        cpu->status.n = cpu->a & 0x80;
        cpu->status.c = cpu->a & 0x40;
        cpu->status.o = ((cpu->a >> 5) ^ (cpu->a >> 6)) & 0x1;
        update_peripherals_cpu(cpu);
    } break;
    case OP_SBX: {
        cpu->cycles++;
        unsigned char val = read_cpu_bus(cpu->bus, operand);
        unsigned char result = cpu->a & cpu->x;
        cpu->x = result - val;
        cpu->status.c = result >= val;
        cpu->status.z = result == val;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
    } break;
    case OP_LXA: {
        cpu->cycles++;
        cpu->a = read_cpu_bus(cpu->bus, operand);
        cpu->x = cpu->a;
        cpu->status.z = cpu->x == 0;
        cpu->status.n = cpu->x & 0x80;
        update_peripherals_cpu(cpu);
    } break;
    case OP_SHY: {
        cpu->cycles++;
        address_t target = operand;
        unsigned char val = cpu->y & ((target >> 8) + 1);

        address_t base = target - cpu->y;
        if ((base & 0xff) + (cpu->y > 0xff)) {
            target = (target & 0xff) | (val << 8);
        }
        write_cpu_bus(cpu->bus, target, val);
        update_peripherals_cpu(cpu);
    } break;
    case OP_SHX: {
        cpu->cycles++;
        address_t target = operand;
        unsigned char val = cpu->x & ((target >> 8) + 1);

        address_t base = target - cpu->x;
        if ((base & 0xff) + (cpu->x > 0xff)) {
            target = (target & 0xff) | (val << 8);
        }
        write_cpu_bus(cpu->bus, target, val);
        update_peripherals_cpu(cpu);
    } break;
    case OP_CLI:
        cpu->cycles++;
        cpu->status.i = false;
        update_peripherals_cpu(cpu);
        break;
    case OP_BRK: {
        cpu->cycles++;
        read_cpu_bus(cpu->bus, cpu->pc++);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        push_stack_cpu(cpu, cpu->pc >> 8);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        push_stack_cpu(cpu, cpu->pc & 0xff);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        cpu->status.b = !cpu->interrupt->irq && !cpu->interrupt->nmi &&
                        !cpu->interrupt->reset;
        push_stack_cpu(cpu, get_status_cpu(cpu));
        cpu->status.b = false;
        cpu->status.i = true;
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char pcl = read_cpu_bus(cpu->bus, cpu->interrupt_vector);
        update_peripherals_cpu(cpu);

        cpu->cycles++;
        unsigned char pch = read_cpu_bus(cpu->bus, cpu->interrupt_vector + 1);
        cpu->pc = (pch << 8) | pcl;
        update_peripherals_cpu(cpu);
    } break;
    default:
        printf("Unimplemented opcode 0x%02X\n", opcode);
        return false;
    }

    // Reset instruction states
    cpu->interrupt_vector = CPU_VEC_IRQ_BRK;

    return true;
}

bool update_cpu(cpu_t *cpu) {
    // Fetch
    unsigned char opcode = fetch_op_cpu(cpu);

    // Decode
    address_t operand = decode_op_cpu(cpu, opcode);

    // Execute
    return execute_op_cpu(cpu, opcode, operand);
}