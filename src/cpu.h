#ifndef CPU_H
#define CPU_H

#include "./apu.h"
#include "./memory.h"
#include "./ppu.h"
#include "./rom.h"

// 6502 has a 16-bit address bus (64k)
#define CPU_RAM_SIZE 1 << 16

// CPU memory map address offsets
#define CPU_MAP_RAM          0x0000
#define CPU_MAP_MIRROR_0     0x0800
#define CPU_MAP_MIRROR_1     0x1000
#define CPU_MAP_MIRROR_2     0x1800
#define CPU_MAP_PPU_REG      0x2000
#define CPU_MAP_PPU_MIRROR   0x2008
#define CPU_MAP_APU_IO       0x4000
#define CPU_MAP_APU_IO_DEBUG 0x4018
#define CPU_MAP_CARTRIDGE    0x4020

// Interrupt vector positions
#define CPU_VEC_NMI     0xfffa
#define CPU_VEC_RESET   0xfffc
#define CPU_VEC_IRQ_BRK 0xfffe

// Memory mapped APU registers
#define APU_REG_PULSE1_0      0x4000
#define APU_REG_PULSE1_1      0x4001
#define APU_REG_PULSE1_2      0x4002
#define APU_REG_PULSE1_3      0x4003
#define APU_REG_PULSE2_0      0x4004
#define APU_REG_PULSE2_1      0x4005
#define APU_REG_PULSE2_2      0x4006
#define APU_REG_PULSE2_3      0x4007
#define APU_REG_TRIANGLE_0    0x4008
#define APU_REG_TRIANGLE_1    0x4009
#define APU_REG_TRIANGLE_2    0x400A
#define APU_REG_TRIANGLE_3    0x400B
#define APU_REG_NOISE_0       0x400C
#define APU_REG_NOISE_1       0x400D
#define APU_REG_NOISE_2       0x400E
#define APU_REG_NOISE_3       0x400F
#define APU_REG_DMC_0         0x4010
#define APU_REG_DMC_1         0x4011
#define APU_REG_DMC_2         0x4012
#define APU_REG_DMC_3         0x4013
#define APU_REG_STATUS        0x4015
#define APU_REG_FRAME_COUNTER 0x4017

// Memory mapped PPU registers
#define PPU_REG_CTRL    0x2000
#define PPU_REG_MASK    0x2001
#define PPU_REG_STATUS  0x2002
#define PPU_REG_OAMADDR 0x2003
#define PPU_REG_OAMDATA 0x2004
#define PPU_REG_SCROLL  0x2005
#define PPU_REG_ADDR    0x2006
#define PPU_REG_DATA    0x2007
#define PPU_REG_OAMDMA  0x4014

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
 * @brief CPU interrupt state.
 *
 */
typedef struct {
    /**
     * @brief IRQ flag.
     *
     */
    bool irq;

    /**
     * @brief NMI flag, not affected by the CPU I-status.
     *
     */
    bool nmi;

    /**
     * @brief Reset flag.
     *
     */
    bool reset;

    /**
     * @brief Vector to the current interrupt handler.
     *
     */
    address_t vector;
} cpu_interrupt_t;

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
     * @brief Current interrupt vector.
     *
     */
    cpu_interrupt_t interrupt;

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

    /**
     * @brief Pointer to the ROM.
     *
     */
    rom_t *rom;

    /**
     * @brief Pointer to the APU.
     *
     */
    apu_t *apu;

    /**
     * @brief Pointer to the PPU.
     *
     */
    ppu_t *ppu;
} cpu_t;

/**
 * @brief Create the CPU.
 *
 * @param rom
 * @param apu
 * @param ppu
 * @return cpu_t*
 */
cpu_t *create_cpu(rom_t *rom, apu_t *apu, ppu_t *ppu);

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
 * @brief Convert mirrored addresses to actual addresses.
 *
 * @param address
 * @return address_t
 */
address_t mirror_address_cpu(address_t address);

/**
 * @brief Apply mapper to address that lies on the ROM cartridge section.
 *
 * @param cpu
 * @param address
 * @return unsigned char*
 */
unsigned char *apply_memory_mapper_cpu(cpu_t *cpu, address_t address);

/**
 * @brief Get the pointer to memory at an address.
 *
 * This is an abstraction to allow accessing the different memory sections
 * available to the CPU (RAM, PRG ROM, CHR ROM, PPU registers, etc.).
 *
 * @param cpu
 * @param address
 * @return unsigned char*
 */
unsigned char *get_memory_cpu(cpu_t *cpu, address_t address);

/**
 * @brief Read a byte from the CPU's memory.
 *
 * @param cpu
 * @param address
 * @return unsigned char
 */
unsigned char read_byte_cpu(cpu_t *cpu, address_t address);

/**
 * @brief Read a short from the CPU's memory.
 *
 * @param cpu
 * @param address
 * @return unsigned short
 */
unsigned short read_short_cpu(cpu_t *cpu, address_t address);

/**
 * @brief Read a short from the CPU's zero-page memory.
 *
 * This handles the wrap-around of the second byte address.
 *
 * @param cpu
 * @param address
 * @return unsigned short
 */
unsigned short read_short_zp_cpu(cpu_t *cpu, unsigned char address);

/**
 * @brief Write a byte to the CPU's memory.
 *
 * @param cpu
 * @param address
 * @param value
 */
void write_byte_cpu(cpu_t *cpu, address_t address, unsigned char value);

/**
 * @brief Write a short to the CPU's memory.
 *
 * @param cpu
 * @param address
 * @param value
 */
void write_short_cpu(cpu_t *cpu, address_t address, unsigned short value);

/**
 * @brief Push a byte onto the stack.
 *
 * @param cpu
 * @param value
 */
void push_byte_cpu(cpu_t *cpu, unsigned char value);

/**
 * @brief Push a short onto the stack.
 *
 * @param cpu
 * @param value
 */
void push_short_cpu(cpu_t *cpu, unsigned short value);

/**
 * @brief Peek a byte from the stack.
 *
 * @param cpu
 * @return unsigned char
 */
unsigned char pop_byte_cpu(cpu_t *cpu);

/**
 * @brief Peek a short from the stack.
 *
 * @param cpu
 * @return unsigned short
 */
unsigned short pop_short_cpu(cpu_t *cpu);

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