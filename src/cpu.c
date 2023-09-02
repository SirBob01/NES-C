#include "./cpu.h"
#include "./ops.h"
#include "cpu_bus.h"
#include "interrupt.h"
#include "memory.h"

void create_cpu(cpu_t *cpu, cpu_bus_t *bus, interrupt_t *interrupt) {
    // Set registers
    cpu->registers.a = 0;
    cpu->registers.x = 0;
    cpu->registers.y = 0;
    cpu->registers.pc = 0;
    cpu->registers.s = 0xfd;
    cpu->registers.p = 0x20 | CPU_STATUS_I;

    // Cycles on reset
    cpu->cycles = 7;
    cpu->state.tick = 0;
    cpu->state.opstate = CPU_OPSTATE_FETCH;

    // Peripherals
    cpu->bus = bus;
    cpu->interrupt = interrupt;
    cpu->interrupt_vector = CPU_VEC_IRQ_BRK;
}

void destroy_cpu(cpu_t *cpu) {}

void set_status_cpu(cpu_t *cpu, unsigned char mask, bool value) {
    cpu->registers.p = (cpu->registers.p & ~mask) | (-value & mask);
}

void set_opstate_cpu(cpu_t *cpu, cpu_opstate_t opstate) {
    cpu->state.opstate = opstate;
    cpu->state.tick = 0;
}

unsigned char read_byte_cpu(cpu_t *cpu, address_t address) {
    operation_t operation = OP_TABLE[cpu->state.opcode];
    if (operation.address_mode == ADDR_ACCUMULATOR) {
        return cpu->registers.a;
    } else {
        return read_byte_cpu_bus(cpu->bus, address);
    }
}

void write_byte_cpu(cpu_t *cpu, address_t address, unsigned char value) {
    operation_t operation = OP_TABLE[cpu->state.opcode];
    if (operation.address_mode == ADDR_ACCUMULATOR) {
        cpu->registers.a = value;
    } else {
        write_byte_cpu_bus(cpu->bus, address, value);
    }
}

void push_byte_cpu(cpu_t *cpu, unsigned char value) {
    write_byte_cpu_bus(cpu->bus, 0x100 | cpu->registers.s, value);
    cpu->registers.s--;
}

unsigned char pull_byte_cpu(cpu_t *cpu) {
    cpu->registers.s++;
    return read_byte_cpu_bus(cpu->bus, 0x100 | cpu->registers.s);
}

void read_state_cpu(cpu_t *cpu, char *buffer, unsigned buffer_size) {
    unsigned char opcode = read_byte_cpu_bus(cpu->bus, cpu->registers.pc);
    operation_t operation = OP_TABLE[opcode];
    unsigned char term_count = ADDRESS_MODE_SIZES[operation.address_mode];
    switch (term_count) {
    case 1:
        snprintf(buffer,
                 buffer_size,
                 "%04X  %02X        A:%02X X:%02X Y:%02X P:%02X SP:%02X "
                 "PPU:%3d,%3d CYC:"
                 "%lu",
                 cpu->registers.pc,
                 opcode,
                 cpu->registers.a,
                 cpu->registers.x,
                 cpu->registers.y,
                 cpu->registers.p,
                 cpu->registers.s,
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
                 cpu->registers.pc,
                 opcode,
                 read_byte_cpu_bus(cpu->bus, cpu->registers.pc + 1),
                 cpu->registers.a,
                 cpu->registers.x,
                 cpu->registers.y,
                 cpu->registers.p,
                 cpu->registers.s,
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
                 cpu->registers.pc,
                 opcode,
                 read_byte_cpu_bus(cpu->bus, cpu->registers.pc + 1),
                 read_byte_cpu_bus(cpu->bus, cpu->registers.pc + 2),
                 cpu->registers.a,
                 cpu->registers.x,
                 cpu->registers.y,
                 cpu->registers.p,
                 cpu->registers.s,
                 cpu->bus->ppu->scanline,
                 cpu->bus->ppu->dot,
                 cpu->cycles);
        break;
    default:
        break;
    }
}

bool is_idle_cpu(cpu_t *cpu) { return cpu->state.opstate == CPU_OPSTATE_FETCH; }

unsigned char read_pc(cpu_t *cpu) {
    return read_byte_cpu_bus(cpu->bus, cpu->registers.pc++);
}

void addr_implied_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.skip_transition = true;
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_immediate_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.address = cpu->registers.pc++;
        cpu->state.skip_transition = true;
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_accumulator_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.skip_transition = true;
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_relative_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        // Address will store the offset
        cpu->state.address = (signed char)(read_pc(cpu));
        cpu->state.skip_transition = true;
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_absolute_cpu(cpu_t *cpu, bool skip_transition) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.address = read_pc(cpu);
        break;
    case 2:
        cpu->state.address |= read_pc(cpu) << 8;
        if (cpu->state.opgroup != CPU_OPGROUP_RW) {
            cpu->state.skip_transition = skip_transition;
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        }
        break;
    case 3:
        cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
        break;
    case 4:
        write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_zero_page_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.address = read_pc(cpu);
        if (cpu->state.opgroup != CPU_OPGROUP_RW) {
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        }
        break;
    case 2:
        cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
        break;
    case 3:
        write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_indirect_x_cpu(cpu_t *cpu) {
    switch (cpu->state.opgroup) {
    case CPU_OPGROUP_RW:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address += cpu->registers.x;
            break;
        case 3:
            cpu->state.address =
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address));
            break;
        case 4:
            cpu->state.address |=
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address + 1))
                << 8;
            break;
        case 5:
            cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
            break;
        case 6:
            write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
            cpu->state.skip_transition = false;
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    default:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address += cpu->registers.x;
            break;
        case 3:
            cpu->state.address =
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address));
            break;
        case 4:
            cpu->state.address |=
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address + 1))
                << 8;
            cpu->state.skip_transition = false;
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    }
}

void addr_indirect_y_cpu(cpu_t *cpu) {
    switch (cpu->state.opgroup) {
    case CPU_OPGROUP_RW:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.address =
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address));
            break;
        case 3:
            cpu->state.address |=
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address + 1))
                << 8;
            break;
        case 4:
            cpu->state.address += cpu->registers.y;
            break;
        case 5:
            cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
            break;
        case 6:
            write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    case CPU_OPGROUP_R:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.address =
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address));
            break;
        case 3:
            cpu->state.address |=
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address + 1))
                << 8;
            cpu->state.base_address = cpu->state.address;
            break;
        case 4:
            cpu->state.address = cpu->state.base_address + cpu->registers.y;
            cpu->state.skip_transition =
                !((cpu->state.base_address & 0xff) + cpu->registers.y > 0xff);
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    default:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.address =
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address));
            break;
        case 3:
            cpu->state.address |=
                read_byte_cpu_bus(cpu->bus,
                                  (unsigned char)(cpu->state.base_address + 1))
                << 8;
            cpu->state.base_address = cpu->state.address;
            break;
        case 4:
            cpu->state.address = cpu->state.base_address + cpu->registers.y;
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    }
}

void addr_indirect_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.base_address =
            read_byte_cpu_bus(cpu->bus, cpu->registers.pc);
        break;
    case 2:
        cpu->state.base_address |=
            read_byte_cpu_bus(cpu->bus, cpu->registers.pc + 1) << 8;
        break;
    case 3:
        cpu->state.address =
            read_byte_cpu_bus(cpu->bus, cpu->state.base_address);
        break;
    case 4: {
        address_t next_addr;
        if ((cpu->state.base_address & 0xff) == 0xff) {
            next_addr = cpu->state.base_address & 0xff00;
        } else {
            next_addr = cpu->state.base_address + 1;
        }
        cpu->state.address |= read_byte_cpu_bus(cpu->bus, next_addr) << 8;
        cpu->state.skip_transition = true;
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
    } break;
    }
}

void addr_absolute_y_cpu(cpu_t *cpu) {
    switch (cpu->state.opgroup) {
    case CPU_OPGROUP_R:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address |= read_pc(cpu) << 8;
            break;
        case 3:
            cpu->state.address = cpu->state.base_address + cpu->registers.y;
            cpu->state.skip_transition =
                !((cpu->state.base_address & 0xff) + cpu->registers.y > 0xff);
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    case CPU_OPGROUP_RW:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address |= read_pc(cpu) << 8;
            break;
        case 3:
            cpu->state.address = cpu->state.base_address + cpu->registers.y;
            break;
        case 4:
            cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
            break;
        case 5:
            write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    default:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address |= read_pc(cpu) << 8;
            break;
        case 3:
            cpu->state.address = cpu->state.base_address + cpu->registers.y;
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    }
}

void addr_zero_page_x_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.base_address = read_pc(cpu);
        break;
    case 2:
        cpu->state.address =
            (unsigned char)(cpu->state.base_address + cpu->registers.x);
        if (cpu->state.opgroup != CPU_OPGROUP_RW) {
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        }
        break;
    case 3:
        cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
        break;
    case 4:
        write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_zero_page_y_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.base_address = read_pc(cpu);
        break;
    case 2:
        cpu->state.address =
            (unsigned char)(cpu->state.base_address + cpu->registers.y);
        if (cpu->state.opgroup != CPU_OPGROUP_RW) {
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        }
        break;
    case 3:
        cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
        break;
    case 4:
        write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    }
}

void addr_absolute_x_cpu(cpu_t *cpu) {
    switch (cpu->state.opgroup) {
    case CPU_OPGROUP_RW:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address |= read_pc(cpu) << 8;
            break;
        case 3:
            cpu->state.address = cpu->state.base_address + cpu->registers.x;
            break;
        case 4:
            cpu->state.tmp = read_byte_cpu(cpu, cpu->state.address);
            break;
        case 5:
            write_byte_cpu(cpu, cpu->state.address, cpu->state.tmp);
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    case CPU_OPGROUP_R:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address |= read_pc(cpu) << 8;
            break;
        case 3:
            cpu->state.address = cpu->state.base_address + cpu->registers.x;
            cpu->state.skip_transition =
                !((cpu->state.base_address & 0xff) + cpu->registers.x > 0xff);
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    default:
        switch (cpu->state.tick) {
        case 1:
            cpu->state.base_address = read_pc(cpu);
            break;
        case 2:
            cpu->state.base_address |= read_pc(cpu) << 8;
            break;
        case 3:
            cpu->state.address = cpu->state.base_address + cpu->registers.x;
            set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
            break;
        }
        break;
    }
}

void op_jmp_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.pc = cpu->state.address;
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_ldx_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.x = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.x == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.x & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_stx_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        write_byte_cpu(cpu, cpu->state.address, cpu->registers.x);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_jsr_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->state.address = read_pc(cpu);
        break;
    case 2:
        // Internal op, not relevant here
        break;
    case 3:
        push_byte_cpu(cpu, cpu->registers.pc >> 8);
        break;
    case 4:
        push_byte_cpu(cpu, cpu->registers.pc & 0xff);
        break;
    case 5:
        cpu->registers.pc = cpu->state.address | (read_pc(cpu) << 8);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_nop_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_sec_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_status_cpu(cpu, CPU_STATUS_C, true);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_bcs_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (cpu->registers.p & CPU_STATUS_C) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_clc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_status_cpu(cpu, CPU_STATUS_C, false);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_bcc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (!(cpu->registers.p & CPU_STATUS_C)) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_lda_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_beq_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (cpu->registers.p & CPU_STATUS_Z) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_bne_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (!(cpu->registers.p & CPU_STATUS_Z)) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_sta_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        write_byte_cpu(cpu, cpu->state.address, cpu->registers.a);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_bit_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, !(cpu->registers.a & val));
        set_status_cpu(cpu, CPU_STATUS_N, val & 0x80);
        set_status_cpu(cpu, CPU_STATUS_O, val & 0x40);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);

    } break;
    }
}

void op_bvs_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (cpu->registers.p & CPU_STATUS_O) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_bvc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (!(cpu->registers.p & CPU_STATUS_O)) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_bpl_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (!(cpu->registers.p & CPU_STATUS_N)) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_rts_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        read_pc(cpu);
        break;
    case 2:
        break;
    case 3:
        cpu->registers.pc = pull_byte_cpu(cpu);
        break;
    case 4:
        cpu->registers.pc |= pull_byte_cpu(cpu) << 8;
        break;
    case 5:
        cpu->registers.pc++;
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_sei_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_status_cpu(cpu, CPU_STATUS_I, true);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_sed_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_status_cpu(cpu, CPU_STATUS_D, true);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_php_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        read_byte_cpu(cpu, cpu->registers.pc);
        break;
    case 2:
        set_status_cpu(cpu, CPU_STATUS_B, true);
        push_byte_cpu(cpu, cpu->registers.p);
        set_status_cpu(cpu, CPU_STATUS_B, false);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_pla_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        read_byte_cpu(cpu, cpu->registers.pc);
        break;
    case 2:
        break;
    case 3:
        cpu->registers.a = pull_byte_cpu(cpu);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_and_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a &= read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_cmp_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char sub = cpu->registers.a - val;
        set_status_cpu(cpu, CPU_STATUS_C, cpu->registers.a >= val);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == val);
        set_status_cpu(cpu, CPU_STATUS_N, sub & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_cld_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_status_cpu(cpu, CPU_STATUS_D, false);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_pha_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        read_byte_cpu(cpu, cpu->registers.pc);
        break;
    case 2:
        push_byte_cpu(cpu, cpu->registers.a);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_plp_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        read_byte_cpu(cpu, cpu->registers.pc);
        break;
    case 2:
        break;
    case 3: {
        unsigned char status = pull_byte_cpu(cpu);
        cpu->registers.p = (cpu->registers.p & 0x30) | (status & 0xcf);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_bmi_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        if (cpu->registers.p & CPU_STATUS_N) {
            cpu->registers.pc += cpu->state.address;
        } else {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
        break;
    case 2: {
        address_t prev_pc = cpu->registers.pc - cpu->state.address;
        if ((cpu->registers.pc & 0xff00) == (prev_pc & 0xff00)) {
            set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        }
    } break;
    case 3:
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_ora_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a |= read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_clv_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_status_cpu(cpu, CPU_STATUS_O, false);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_eor_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a ^= read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_adc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char m = read_byte_cpu(cpu, cpu->state.address);
        unsigned char n = cpu->registers.a;
        unsigned short sum = m + n + (cpu->registers.p & 1);
        cpu->registers.a = sum;
        set_status_cpu(cpu, CPU_STATUS_C, sum > 0xff);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_status_cpu(
            cpu,
            CPU_STATUS_O,
            ((m ^ cpu->registers.a) & (n ^ cpu->registers.a) & 0x80) > 0);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_ldy_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.y = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.y == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.y & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_cpy_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char sub = cpu->registers.y - val;
        set_status_cpu(cpu, CPU_STATUS_C, cpu->registers.y >= val);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.y == val);
        set_status_cpu(cpu, CPU_STATUS_N, sub & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_cpx_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char sub = cpu->registers.x - val;
        set_status_cpu(cpu, CPU_STATUS_C, cpu->registers.x >= val);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.x == val);
        set_status_cpu(cpu, CPU_STATUS_N, sub & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_sbc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char m = ~read_byte_cpu(cpu, cpu->state.address);
        unsigned char n = cpu->registers.a;
        unsigned short diff = n + m + (cpu->registers.p & 1);
        cpu->registers.a = diff;
        set_status_cpu(cpu, CPU_STATUS_C, diff > 0xff);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_status_cpu(
            cpu,
            CPU_STATUS_O,
            ((m ^ cpu->registers.a) & (n ^ cpu->registers.a) & 0x80) > 0);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_iny_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.y++;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.y == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.y & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_inx_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.x++;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.x == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.x & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_dey_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.y--;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.y == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.y & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_dex_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.x--;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.x == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.x & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_tay_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.y = cpu->registers.a;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.y == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.y & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_tax_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.x = cpu->registers.a;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.x == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.x & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_tya_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a = cpu->registers.y;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_txa_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a = cpu->registers.x;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_tsx_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.x = cpu->registers.s;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.x == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.x & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_txs_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.s = cpu->registers.x;
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_rti_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        read_pc(cpu);
        break;
    case 2:
        break;
    case 3:
        cpu->registers.p = pull_byte_cpu(cpu) | 0x20;
        break;
    case 4:
        cpu->registers.pc = pull_byte_cpu(cpu);
        break;
    case 5:
        cpu->registers.pc |= pull_byte_cpu(cpu) << 8;
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_lsr_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x1);
        val >>= 1;
        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, false);
        write_byte_cpu(cpu, cpu->state.address, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_asl_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x80);
        val <<= 1;
        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, val >> 7);
        write_byte_cpu(cpu, cpu->state.address, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_ror_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char old_c = cpu->registers.p & CPU_STATUS_C;
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x1);
        val >>= 1;
        val |= (old_c << 7);

        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, val >> 7);
        write_byte_cpu(cpu, cpu->state.address, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_rol_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char old_c = cpu->registers.p & CPU_STATUS_C;
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x80);
        val <<= 1;
        val |= old_c;

        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, val >> 7);
        write_byte_cpu(cpu, cpu->state.address, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_sty_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        write_byte_cpu(cpu, cpu->state.address, cpu->registers.y);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_inc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        val++;
        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, val & 0x80);
        write_byte_cpu(cpu, cpu->state.address, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_dec_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        val--;
        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, val & 0x80);
        write_byte_cpu(cpu, cpu->state.address, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_lax_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a = read_byte_cpu(cpu, cpu->state.address);
        cpu->registers.x = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_sax_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        write_byte_cpu(cpu,
                       cpu->state.address,
                       cpu->registers.a & cpu->registers.x);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_dcp_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // DEC
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        val--;
        write_byte_cpu(cpu, cpu->state.address, val);

        // CMP
        unsigned char sub = cpu->registers.a - val;
        set_status_cpu(cpu, CPU_STATUS_C, cpu->registers.a >= val);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == val);
        set_status_cpu(cpu, CPU_STATUS_N, sub & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_isc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // INC
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        val++;
        write_byte_cpu(cpu, cpu->state.address, val);

        // SBC
        unsigned char m = ~val;
        unsigned char n = cpu->registers.a;
        unsigned short diff = n + m + (cpu->registers.p & 1);
        cpu->registers.a = diff;
        set_status_cpu(cpu, CPU_STATUS_C, diff > 0xff);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_status_cpu(
            cpu,
            CPU_STATUS_O,
            ((m ^ cpu->registers.a) & (n ^ cpu->registers.a) & 0x80) > 0);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_slo_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // ASL
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x80);
        val <<= 1;
        write_byte_cpu(cpu, cpu->state.address, val);

        // ORA
        cpu->registers.a |= read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_rla_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // ROL
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char old_c = cpu->registers.p & CPU_STATUS_C;
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x80);
        val <<= 1;
        val |= old_c;
        write_byte_cpu(cpu, cpu->state.address, val);

        // AND
        cpu->registers.a &= val;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    }
    }
}

void op_sre_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // LSR
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x1);
        val >>= 1;
        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, false);
        write_byte_cpu(cpu, cpu->state.address, val);

        // EOR
        cpu->registers.a ^= val;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    }
    }
}

void op_rra_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // ROR
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char old_c = cpu->registers.p & CPU_STATUS_C;
        set_status_cpu(cpu, CPU_STATUS_C, val & 0x1);
        val >>= 1;
        val |= (old_c << 7);

        set_status_cpu(cpu, CPU_STATUS_Z, val == 0);
        set_status_cpu(cpu, CPU_STATUS_N, val >> 7);
        write_byte_cpu(cpu, cpu->state.address, val);

        // ADC
        unsigned char m = read_byte_cpu(cpu, cpu->state.address);
        unsigned char n = cpu->registers.a;
        unsigned short sum = m + n + (cpu->registers.p & 1);
        cpu->registers.a = sum;
        set_status_cpu(cpu, CPU_STATUS_C, sum > 0xff);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_status_cpu(
            cpu,
            CPU_STATUS_O,
            ((m ^ cpu->registers.a) & (n ^ cpu->registers.a) & 0x80) > 0);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    }
    }
}

void op_cli_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        set_status_cpu(cpu, CPU_STATUS_I, false);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_anc_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // AND
        cpu->registers.a &= read_byte_cpu(cpu, cpu->state.address);
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);

        // Set C
        set_status_cpu(cpu, CPU_STATUS_C, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_alr_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        // AND
        cpu->registers.a &= read_byte_cpu(cpu, cpu->state.address);

        // LSR
        set_status_cpu(cpu, CPU_STATUS_C, cpu->registers.a & 0x1);
        cpu->registers.a >>= 1;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, false);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_arr_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        cpu->registers.a &= read_byte_cpu(cpu, cpu->state.address);
        cpu->registers.a >>= 1;
        cpu->registers.a |= (cpu->registers.p & CPU_STATUS_C) << 7;

        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);

        set_status_cpu(cpu, CPU_STATUS_C, (cpu->registers.a >> 6) & 0x1);
        set_status_cpu(cpu,
                       CPU_STATUS_O,
                       ((cpu->registers.a >> 5) ^ (cpu->registers.a >> 6)) &
                           0x1);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_lxa_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        cpu->registers.a = read_byte_cpu(cpu, cpu->state.address);
        cpu->registers.x = cpu->registers.a;
        set_status_cpu(cpu, CPU_STATUS_Z, cpu->registers.a == 0);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.a & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void op_sbx_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        unsigned char val = read_byte_cpu(cpu, cpu->state.address);
        unsigned char and_res = cpu->registers.a & cpu->registers.x;
        cpu->registers.x = and_res - val;
        set_status_cpu(cpu, CPU_STATUS_C, and_res >= val);
        set_status_cpu(cpu, CPU_STATUS_Z, and_res == val);
        set_status_cpu(cpu, CPU_STATUS_N, cpu->registers.x & 0x80);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_shy_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        address_t target = cpu->state.address;
        unsigned char val = cpu->registers.y & ((target >> 8) + 1);

        address_t base = target - cpu->registers.y;
        if ((base & 0xff) + (cpu->registers.y > 0xff)) {
            target = (target & 0xff) | (val << 8);
        }
        write_byte_cpu(cpu, target, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_shx_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1: {
        address_t target = cpu->state.address;
        unsigned char val = cpu->registers.x & ((target >> 8) + 1);

        address_t base = target - cpu->registers.x;
        if ((base & 0xff) + (cpu->registers.x > 0xff)) {
            target = (target & 0xff) | (val << 8);
        }
        write_byte_cpu(cpu, target, val);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
    } break;
    }
}

void op_brk_cpu(cpu_t *cpu) {
    switch (cpu->state.tick) {
    case 1:
        read_pc(cpu);
        break;
    case 2:
        push_byte_cpu(cpu, cpu->registers.pc >> 8);
        break;
    case 3:
        push_byte_cpu(cpu, cpu->registers.pc & 0xff);
        break;
    case 4:
        push_byte_cpu(cpu, cpu->registers.p | CPU_STATUS_B);
        break;
    case 5:
        cpu->registers.pc = read_byte_cpu(cpu, CPU_VEC_IRQ_BRK);
        break;
    case 6:
        cpu->registers.pc |= read_byte_cpu(cpu, CPU_VEC_IRQ_BRK + 1) << 8;
        set_status_cpu(cpu, CPU_STATUS_I, true);
        set_opstate_cpu(cpu, CPU_OPSTATE_FETCH);
        break;
    }
}

void fetch_cpu(cpu_t *cpu) {
    // Handle NMI, IRQ, and RESET interrupts
    if (cpu->interrupt->nmi) {
        cpu->interrupt_vector = CPU_VEC_NMI;
        cpu->state.opcode = 0;
    } else if (cpu->interrupt->irq && !(cpu->registers.p & CPU_STATUS_I)) {
        cpu->interrupt_vector = CPU_VEC_IRQ_BRK;
        cpu->state.opcode = 0;
    } else if (cpu->interrupt->reset && !(cpu->registers.p & CPU_STATUS_I)) {
        cpu->interrupt_vector = CPU_VEC_RESET;
        cpu->state.opcode = 0;
    } else {
        // No interrupts, next instruction from program counter
        cpu->interrupt_vector = CPU_VEC_IRQ_BRK;
        cpu->state.opcode = read_pc(cpu);
    }

    operation_t operation = OP_TABLE[cpu->state.opcode];
    switch (operation.mnemonic) {
    case OP_ASL:
    case OP_LSR:
    case OP_ROL:
    case OP_ROR:
    case OP_INC:
    case OP_DEC:
    case OP_SLO:
    case OP_SRE:
    case OP_RLA:
    case OP_RRA:
    case OP_ISC:
    case OP_DCP:
        cpu->state.opgroup = CPU_OPGROUP_RW;
        break;
    case OP_LDA:
    case OP_LDX:
    case OP_LDY:
    case OP_EOR:
    case OP_AND:
    case OP_ORA:
    case OP_ADC:
    case OP_SBC:
    case OP_CMP:
    case OP_BIT:
    case OP_LAX:
    case OP_NOP:
        cpu->state.opgroup = CPU_OPGROUP_R;
        break;
    case OP_STA:
    case OP_STX:
    case OP_STY:
    case OP_SAX:
        cpu->state.opgroup = CPU_OPGROUP_W;
        break;
    default:
        cpu->state.opgroup = CPU_OPGROUP_NONE;
        break;
    }

    switch (operation.mnemonic) {
    case OP_JSR:
    case OP_BRK:
    case OP_RTI:
    case OP_RTS:
    case OP_PHA:
    case OP_PHP:
    case OP_PLA:
    case OP_PLP:
        set_opstate_cpu(cpu, CPU_OPSTATE_EXECUTE);
        break;
    default:
        set_opstate_cpu(cpu, CPU_OPSTATE_DECODE);
        break;
    }
}

void decode_cpu(cpu_t *cpu) {
    operation_t operation = OP_TABLE[cpu->state.opcode];
    reset_interrupt(cpu->interrupt);
    switch (operation.address_mode) {
    case ADDR_ABSOLUTE:
        addr_absolute_cpu(cpu, operation.mnemonic == OP_JMP);
        break;
    case ADDR_IMMEDIATE:
        addr_immediate_cpu(cpu);
        break;
    case ADDR_ZERO_PAGE:
        addr_zero_page_cpu(cpu);
        break;
    case ADDR_IMPLIED:
        addr_implied_cpu(cpu);
        break;
    case ADDR_RELATIVE:
        addr_relative_cpu(cpu);
        break;
    case ADDR_ACCUMULATOR:
        addr_accumulator_cpu(cpu);
        break;
    case ADDR_INDIRECT_X:
        addr_indirect_x_cpu(cpu);
        break;
    case ADDR_INDIRECT_Y:
        addr_indirect_y_cpu(cpu);
        break;
    case ADDR_INDIRECT:
        addr_indirect_cpu(cpu);
        break;
    case ADDR_ABSOLUTE_Y:
        addr_absolute_y_cpu(cpu);
        break;
    case ADDR_ZERO_PAGE_X:
        addr_zero_page_x_cpu(cpu);
        break;
    case ADDR_ZERO_PAGE_Y:
        addr_zero_page_y_cpu(cpu);
        break;
    case ADDR_ABSOLUTE_X:
        addr_absolute_x_cpu(cpu);
        break;
    default:
        printf("Error: Unhandled addressing mode\n");
        exit(1);
    }
}

void execute_cpu(cpu_t *cpu) {
    operation_t operation = OP_TABLE[cpu->state.opcode];
    switch (operation.mnemonic) {
    case OP_JMP:
        op_jmp_cpu(cpu);
        break;
    case OP_LDX:
        op_ldx_cpu(cpu);
        break;
    case OP_STX:
        op_stx_cpu(cpu);
        break;
    case OP_JSR:
        op_jsr_cpu(cpu);
        break;
    case OP_NOP:
        op_nop_cpu(cpu);
        break;
    case OP_SEC:
        op_sec_cpu(cpu);
        break;
    case OP_BCS:
        op_bcs_cpu(cpu);
        break;
    case OP_CLC:
        op_clc_cpu(cpu);
        break;
    case OP_BCC:
        op_bcc_cpu(cpu);
        break;
    case OP_LDA:
        op_lda_cpu(cpu);
        break;
    case OP_BEQ:
        op_beq_cpu(cpu);
        break;
    case OP_BNE:
        op_bne_cpu(cpu);
        break;
    case OP_STA:
        op_sta_cpu(cpu);
        break;
    case OP_BIT:
        op_bit_cpu(cpu);
        break;
    case OP_BVS:
        op_bvs_cpu(cpu);
        break;
    case OP_BVC:
        op_bvc_cpu(cpu);
        break;
    case OP_BPL:
        op_bpl_cpu(cpu);
        break;
    case OP_RTS:
        op_rts_cpu(cpu);
        break;
    case OP_SEI:
        op_sei_cpu(cpu);
        break;
    case OP_SED:
        op_sed_cpu(cpu);
        break;
    case OP_PHP:
        op_php_cpu(cpu);
        break;
    case OP_PLA:
        op_pla_cpu(cpu);
        break;
    case OP_AND:
        op_and_cpu(cpu);
        break;
    case OP_CMP:
        op_cmp_cpu(cpu);
        break;
    case OP_CLD:
        op_cld_cpu(cpu);
        break;
    case OP_PHA:
        op_pha_cpu(cpu);
        break;
    case OP_PLP:
        op_plp_cpu(cpu);
        break;
    case OP_BMI:
        op_bmi_cpu(cpu);
        break;
    case OP_ORA:
        op_ora_cpu(cpu);
        break;
    case OP_CLV:
        op_clv_cpu(cpu);
        break;
    case OP_EOR:
        op_eor_cpu(cpu);
        break;
    case OP_ADC:
        op_adc_cpu(cpu);
        break;
    case OP_LDY:
        op_ldy_cpu(cpu);
        break;
    case OP_CPY:
        op_cpy_cpu(cpu);
        break;
    case OP_CPX:
        op_cpx_cpu(cpu);
        break;
    case OP_SBC:
        op_sbc_cpu(cpu);
        break;
    case OP_INY:
        op_iny_cpu(cpu);
        break;
    case OP_INX:
        op_inx_cpu(cpu);
        break;
    case OP_DEY:
        op_dey_cpu(cpu);
        break;
    case OP_DEX:
        op_dex_cpu(cpu);
        break;
    case OP_TAY:
        op_tay_cpu(cpu);
        break;
    case OP_TAX:
        op_tax_cpu(cpu);
        break;
    case OP_TYA:
        op_tya_cpu(cpu);
        break;
    case OP_TXA:
        op_txa_cpu(cpu);
        break;
    case OP_TSX:
        op_tsx_cpu(cpu);
        break;
    case OP_TXS:
        op_txs_cpu(cpu);
        break;
    case OP_RTI:
        op_rti_cpu(cpu);
        break;
    case OP_LSR:
        op_lsr_cpu(cpu);
        break;
    case OP_ASL:
        op_asl_cpu(cpu);
        break;
    case OP_ROR:
        op_ror_cpu(cpu);
        break;
    case OP_ROL:
        op_rol_cpu(cpu);
        break;
    case OP_STY:
        op_sty_cpu(cpu);
        break;
    case OP_INC:
        op_inc_cpu(cpu);
        break;
    case OP_DEC:
        op_dec_cpu(cpu);
        break;
    case OP_LAX:
        op_lax_cpu(cpu);
        break;
    case OP_SAX:
        op_sax_cpu(cpu);
        break;
    case OP_DCP:
        op_dcp_cpu(cpu);
        break;
    case OP_ISC:
        op_isc_cpu(cpu);
        break;
    case OP_SLO:
        op_slo_cpu(cpu);
        break;
    case OP_RLA:
        op_rla_cpu(cpu);
        break;
    case OP_SRE:
        op_sre_cpu(cpu);
        break;
    case OP_RRA:
        op_rra_cpu(cpu);
        break;
    case OP_CLI:
        op_cli_cpu(cpu);
        break;
    case OP_ANC:
        op_anc_cpu(cpu);
        break;
    case OP_ALR:
        op_alr_cpu(cpu);
        break;
    case OP_ARR:
        op_arr_cpu(cpu);
        break;
    case OP_LXA:
        op_lxa_cpu(cpu);
        break;
    case OP_SBX:
        op_sbx_cpu(cpu);
        break;
    case OP_SHY:
        op_shy_cpu(cpu);
        break;
    case OP_SHX:
        op_shx_cpu(cpu);
        break;
    case OP_BRK:
        op_brk_cpu(cpu);
        break;
    case OP_JAM:
        fprintf(stderr, "JAM opcode encountered: 0x%02X\n", cpu->state.opcode);
        exit(1);
    default:
        fprintf(stderr,
                "Error: Unknown CPU opcode $%02X at $%04X\n",
                cpu->state.opcode,
                cpu->registers.pc);
        exit(1);
    }
}

void update_cpu(cpu_t *cpu) {
    cpu->cycles++;
    cpu->state.tick++;

    // Process instruction
    switch (cpu->state.opstate) {
    case CPU_OPSTATE_FETCH:
        fetch_cpu(cpu);
        break;
    case CPU_OPSTATE_DECODE:
        decode_cpu(cpu);
        break;
    case CPU_OPSTATE_EXECUTE:
        execute_cpu(cpu);
        break;
    }

    // Skip the transition between decode and execute
    if (cpu->state.skip_transition &&
        cpu->state.opstate == CPU_OPSTATE_EXECUTE) {
        cpu->state.skip_transition = false;
        cpu->state.tick++;
        execute_cpu(cpu);
    }
}
