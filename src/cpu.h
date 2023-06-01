#ifndef CPU_H
#define CPU_H

#include "./memory.h"
#include "./rom.h"

// 6502 has a 16-bit address bus (64k)
#define CPU_RAM_SIZE 1 << 16

// Indices to the CPU_MEMORY_MAP array
#define CPU_MAP_RAM          0
#define CPU_MAP_MIRROR_0     1
#define CPU_MAP_MIRROR_1     2
#define CPU_MAP_MIRROR_2     3
#define CPU_MAP_PPU_REG      4
#define CPU_MAP_PPU_MIRROR   5
#define CPU_MAP_APU_IO       6
#define CPU_MAP_APU_IO_DEBUG 7
#define CPU_MAP_CARTRIDGE    8

// Interrupt vector positions
#define CPU_VEC_NMI     0xfffa
#define CPU_VEC_RESET   0xfffc
#define CPU_VEC_IRQ_BRK 0xfffe

/**
 * @brief CPU memory map offsets.
 *
 */
static const unsigned CPU_MEMORY_MAP[] = {
    0x0000,  // 2k RAM
    0x0800,  // Mirror 0
    0x1000,  // Mirror 1
    0x1800,  // Mirror 2
    0x2000,  // PPU registers
    0x2008,  // Mirror of 0x2000-0x2007 (every 8 bytes)
    0x4000,  // APU I/O
    0x4018,  // APU I/O (debug)
    0x4020,  // Cartridge
    0x10000, // End-of-memory
};

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
 * @brief Get the true address, accounting for the ROM mapping mode and
 * mirrored memory segments.
 *
 * @param short
 * @return unsigned short
 */
unsigned short map_address_cpu(rom_t *rom, unsigned short address);

/**
 * @brief Read from the CPU memory.
 *
 * @param cpu
 * @param rom
 * @param address
 * @return unsigned char
 */
unsigned char read_cpu(cpu_t *cpu, rom_t *rom, unsigned short address);

/**
 * @brief Write to the CPU memory.
 *
 * @param cpu
 * @param rom
 * @param address
 * @param data
 */
void write_cpu(cpu_t *cpu,
               rom_t *rom,
               unsigned short address,
               unsigned char data);

/**
 * @brief Destroy the CPU.
 *
 * @param cpu
 */
void destroy_cpu(cpu_t *cpu);

#endif