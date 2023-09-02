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
 * @brief CPU operation state.
 *
 */
typedef enum {
    CPU_OPSTATE_FETCH,
    CPU_OPSTATE_DECODE,
    CPU_OPSTATE_EXECUTE
} cpu_opstate_t;

/**
 * @brief CPU operation group.
 *
 */
typedef enum {
    CPU_OPGROUP_NONE,
    CPU_OPGROUP_R,
    CPU_OPGROUP_W,
    CPU_OPGROUP_RW,
} cpu_opgroup_t;

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
 * @brief CPU runtime state.
 *
 */
typedef struct {
    /**
     * @brief Current sub-operation tick.
     *
     */
    unsigned long tick;

    /**
     * @brief Skip the transition cycle between decode and execute.
     *
     */
    bool skip_transition;

    /**
     * @brief Current operation state.
     *
     */
    cpu_opstate_t opstate;

    /**
     * @brief Current operation group.
     *
     */
    cpu_opgroup_t opgroup;

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

    /**
     * @brief Base operand address for indexed addressing.
     *
     */
    address_t base_address;

    /**
     * @brief Temporary value.
     *
     */
    unsigned char tmp;
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
 * @brief Set a status flag of the CPU.
 *
 * @param cpu
 * @param mask
 * @param value
 */
void set_status_cpu(cpu_t *cpu, unsigned char mask, bool value);

/**
 * @brief Read a byte from an address in the CPU bus, unless the current
 * instruction is of the accumulator addressing mode.
 *
 * @param cpu
 * @param address
 * @return unsigned char
 */
unsigned char read_byte_cpu(cpu_t *cpu, address_t address);

/**
 * @brief Write a byte to an address in the CPU bus, unless the current
 * instruction is of the accumulator addressing mode.
 *
 * @param cpu
 * @param address
 * @param value
 */
void write_byte_cpu(cpu_t *cpu, address_t address, unsigned char value);

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