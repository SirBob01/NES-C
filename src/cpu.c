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

void destroy_cpu(cpu_t *cpu) { free_memory(&cpu->memory); }