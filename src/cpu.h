#ifndef CPU_H
#define CPU_H

#include "./cpu_bus.h"
#include "./interrupt.h"
#include "./memory.h"

// Interrupt vector positions
#define CPU_VEC_NMI     0xfffa
#define CPU_VEC_RESET   0xfffc
#define CPU_VEC_IRQ_BRK 0xfffe

// Status flag masks
#define CPU_STATUS_C (1 << 0)
#define CPU_STATUS_Z (1 << 1)
#define CPU_STATUS_I (1 << 2)
#define CPU_STATUS_D (1 << 3)
#define CPU_STATUS_B (1 << 4)
#define CPU_STATUS_O (1 << 6)
#define CPU_STATUS_N (1 << 7)

/**
 * @brief CPU registers.
 *
 */
typedef struct {
    /**
     * @brief Accumulator.
     *
     */
    unsigned char a;

    /**
     * @brief Index registers.
     *
     */
    unsigned char x, y;

    /**
     * @brief Program counter.
     *
     */
    address_t pc;

    /**
     * @brief Stack pointer.
     *
     */
    unsigned char s;

    /**
     * @brief Status flags.
     *
     */
    unsigned char p;
} cpu_registers_t;

/**
 * @brief CPU instruction state.
 *
 */
typedef struct {
    /**
     * @brief Current sub-operation tick.
     *
     */
    unsigned long tick;

    /**
     * @brief Is currently fetching operand?
     *
     */
    bool fetch_operand;

    /**
     * @brief Current opcode.
     *
     */
    unsigned char opcode;

    /**
     * @brief Current operand address.
     *
     */
    address_t address;
} cpu_state_t;

/**
 * @brief 6502 central processing unit.
 *
 */
typedef struct {
    /**
     * @brief Registers.
     *
     */
    cpu_registers_t registers;

    /**
     * @brief Instruction state.
     *
     */
    cpu_state_t state;

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
     * @brief Interrupt controller.
     *
     */
    interrupt_t *interrupt;

    /**
     * @brief Vector to the current interrupt handler.
     *
     */
    address_t interrupt_vector;
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
 * @brief Push a byte onto the stack.
 *
 * @param cpu
 * @param value
 */
void push_byte_cpu(cpu_t *cpu, unsigned char value);

/**
 * @brief Pull a byte from the stack.
 *
 * @param cpu
 * @return unsigned char
 */
unsigned char pull_byte_cpu(cpu_t *cpu);

/**
 * @brief Read the current state of the CPU for debugging.
 *
 * @param cpu
 * @param buffer
 * @param buffer_size
 */
void read_state_cpu(cpu_t *cpu, char *buffer, unsigned buffer_size);

/**
 * @brief Check if the CPU is idle, the state between instructions.
 *
 * @param cpu
 * @return true
 * @return false
 */
bool is_idle_cpu(cpu_t *cpu);

/**
 * @brief Update the CPU.
 *
 * @param cpu
 */
void update_cpu(cpu_t *cpu);

#endif