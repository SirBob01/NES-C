#ifndef OPCODE_H
#define OPCODE_H

#include "./cpu.h"

// Define all 151 possible opcodes
#define ADC_I  0x69
#define ADC_Z  0x65
#define ADC_ZX 0x75
#define ADC_A  0x6d
#define ADC_AX 0x7d
#define ADC_AY 0x79
#define ADC_IX 0x61
#define ADC_IY 0x71

#define JMP_A  0x4c
#define JMP_I  0x6c

// 7  bit  0
// ---- ----
// NVss DIZC
// |||| ||||
// |||| |||+- Carry
// |||| ||+-- Zero
// |||| |+--- Interrupt Disable
// |||| +---- Decimal
// ||++------ No CPU effect, see: the B flag
// |+-------- Overflow
// +--------- Negative

inline void jmp_a(cpu_t *cpu, unsigned char pc0, unsigned char pc1) {
    cpu->registers.pc = (pc1 << 8) | pc0;
    cpu->cycles += 3;
}

inline void adc_i(cpu_t *cpu, unsigned char memory) {
    unsigned char carry = cpu->registers.p & 0x01;
    unsigned short result = cpu->registers.a + memory + carry;
    cpu->registers.p ^= ~(result >> 7);
    cpu->registers.a += result;
    cpu->cycles += 3;
}

#endif