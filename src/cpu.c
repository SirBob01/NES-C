#include "./cpu.h"
#include "./ops.h"

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

    // Peripherals
    cpu->bus = bus;
    cpu->interrupt = interrupt;
    cpu->interrupt_vector = CPU_VEC_IRQ_BRK;
}

void destroy_cpu(cpu_t *cpu) {}

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

unsigned char fetch_opcode_cpu(cpu_t *cpu) {
    // Handle NMI, IRQ, and RESET interrupts
    if (cpu->interrupt->nmi) {
        cpu->interrupt_vector = CPU_VEC_NMI;
        return 0;
    }
    if (cpu->interrupt->irq && !(cpu->registers.p & CPU_STATUS_I)) {
        cpu->interrupt_vector = CPU_VEC_IRQ_BRK;
        return 0;
    }
    if (cpu->interrupt->reset && !(cpu->registers.p & CPU_STATUS_I)) {
        cpu->interrupt_vector = CPU_VEC_RESET;
        return 0;
    }

    // No interrupts, next instruction from program counter
    return read_byte_cpu_bus(cpu->bus, cpu->registers.pc);
}

void update_cpu(cpu_t *cpu) {
    // Fetch opcode
    unsigned char opcode = fetch_opcode_cpu(cpu);
}
