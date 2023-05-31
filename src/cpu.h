#ifndef CPU_H
#define CPU_H

#define CPU_RAM_SIZE 1 << 16

#include "./memory.h"
#include "./rom.h"

/**
 * @brief Registers of the 6502 processor.
 *
 */
typedef struct {
    /**
     * @brief Accumulator.
     *
     */
    unsigned char a;

    /**
     * @brief X and Y are index registers used for several addressing modes.
     *
     */
    unsigned char x, y;

    /**
     * @brief Program counter (2-bytes wide).
     *
     */
    unsigned short pc;

    /**
     * @brief Stack pointer.
     *
     */
    unsigned char s;

    /**
     * @brief Status register.
     *
     */
    unsigned char p;
} cpu_registers_t;

/**
 * @brief CPU emulation state.
 *
 */
typedef struct {
    cpu_registers_t registers;

    /**
     * @brief Number of cycles.
     *
     */
    unsigned long cycles;

    /**
     * @brief Internal CPU memory.
     *
     */
    memory_t memory;
} cpu_t;

/**
 * @brief Create the CPU emualator.
 *
 * @return cpu_t
 */
cpu_t create_cpu();

/**
 * @brief Destroy the CPU.
 *
 * @param cpu
 */
void destroy_cpu(cpu_t *cpu);

#endif