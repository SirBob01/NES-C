#ifndef CPU_H
#define CPU_H

#include "./apu.h"
#include "./cpu_bus.h"
#include "./interrupt.h"
#include "./memory.h"
#include "./ppu.h"

// Interrupt vector positions
#define CPU_VEC_NMI     0xfffa
#define CPU_VEC_RESET   0xfffc
#define CPU_VEC_IRQ_BRK 0xfffe

/**
 * @brief CPU status flags.
 *
 */
typedef struct {
    bool c; // Carry
    bool z; // Zero
    bool i; // Interrupt disable
    bool d; // Decimal mode
    bool b; // Break
    bool o; // Overflow
    bool n; // Negative
} cpu_status_t;

/**
 * @brief CPU emulation state.
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
    address_t pc;

    /**
     * @brief Stack pointer.
     *
     */
    unsigned char s;

    /**
     * @brief CPU status flags.
     *
     */
    cpu_status_t status;

    /**
     * @brief Number of cycles.
     *
     */
    unsigned long cycles;

    /**
     * @brief Pointer to the CPU bus.
     *
     */
    cpu_bus_t *bus;

    /**
     * @brief Interrupt signal controller.
     *
     */
    interrupt_t *interrupt;

    /**
     * @brief NMI assertion flag to delay until next instruction.
     *
     */
    bool nmi_assert;
} cpu_t;

/**
 * @brief Create the CPU.
 *
 * @param cpu
 * @param bus
 * @param interrupt
 */
void create_cpu(cpu_t *cpu, cpu_bus_t *bus, interrupt_t *interrupt);

/**
 * @brief Destroy the CPU.
 *
 * @param cpu
 */
void destroy_cpu(cpu_t *cpu);

/**
 * @brief Get the CPU status flag, useful for debugging.
 *
 * @param cpu
 * @return unsigned char
 */
unsigned char get_status_cpu(cpu_t *cpu);

/**
 * @brief Push a byte onto the CPU stack.
 *
 * @param cpu
 * @param value
 */
void push_stack_cpu(cpu_t *cpu, unsigned char value);

/**
 * @brief Peek a byte from the CPU stack.
 *
 * @param cpu
 * @return unsigned char
 */
unsigned char pop_stack_cpu(cpu_t *cpu);

/**
 * @brief Read the current state of the CPU for debugging.
 *
 * @param cpu
 * @param buffer
 * @param buffer_size
 */
void read_state_cpu(cpu_t *cpu, char *buffer, unsigned buffer_size);

/**
 * @brief Update the CPU.
 *
 * @param cpu
 * @return true
 * @return false
 */
bool update_cpu(cpu_t *cpu);

#endif