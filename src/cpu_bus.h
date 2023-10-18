#ifndef CPU_BUS_H
#define CPU_BUS_H

#include "./apu.h"
#include "./controller.h"
#include "./mapper.h"
#include "./ppu.h"
#include "./rom.h"

// 6502 has a 16-bit address bus (64k)
#define CPU_RAM_SIZE (1 << 16)

// CPU memory map address offsets
#define CPU_MAP_START        0x0000
#define CPU_MAP_MIRROR_0     0x0800
#define CPU_MAP_MIRROR_1     0x1000
#define CPU_MAP_MIRROR_2     0x1800
#define CPU_MAP_PPU_REG      0x2000
#define CPU_MAP_PPU_MIRROR   0x2008
#define CPU_MAP_APU_IO       0x4000
#define CPU_MAP_APU_IO_DEBUG 0x4018
#define CPU_MAP_CARTRIDGE    0x4020
#define CPU_MAP_RAM          0x6000 // Implicitly defined by iNES format
#define CPU_MAP_ROM          0x8000

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

// Memory mapped controller registers
#define CTRL_REG_JOYPAD1 0x4016
#define CTRL_REG_JOYPAD2 0x4017

/**
 * @brief Bus for memory accessing and communication between the CPU and its
 * peripherals.
 *
 */
typedef struct {
    /**
     * @brief CPU memory address space.
     *
     */
    unsigned char memory[CPU_RAM_SIZE];

    /**
     * @brief Internal buffer for reading from PPU data register.
     *
     */
    unsigned char buffer2007;

    /**
     * @brief Pointer to the ROM.
     *
     */
    rom_t *rom;

    /**
     * @brief Pointer to the mapper.
     *
     */
    mapper_t *mapper;

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

    /**
     * @brief Pointer to the input controller.
     *
     */
    controller_t *controller;
} cpu_bus_t;

/**
 * @brief Create the CPU bus.
 *
 * @param bus
 * @param rom
 * @param mapper
 * @param apu
 * @param ppu
 * @param controller
 */
void create_cpu_bus(cpu_bus_t *bus,
                    rom_t *rom,
                    mapper_t *mapper,
                    apu_t *apu,
                    ppu_t *ppu,
                    controller_t *controller);

/**
 * @brief Mirror an address according to the CPU memory map.
 *
 * @param address
 * @return address_t
 */
address_t mirror_cpu_bus(address_t address);

/**
 * @brief Read a byte from the CPU's memory map.
 *
 * @param bus
 * @param address
 * @return unsigned char
 */
unsigned char read_cpu_bus(cpu_bus_t *bus, address_t address);

/**
 * @brief Write a byte to the CPU's memory map.
 *
 * @param bus
 * @param address
 * @param value
 */
void write_cpu_bus(cpu_bus_t *bus, address_t address, unsigned char value);

/**
 * @brief Read a string from the CPU bus.
 *
 * @param bus
 * @param address
 * @param dst
 * @param n
 * @return unsigned
 */
unsigned
read_string_cpu_bus(cpu_bus_t *bus, address_t address, char *dst, unsigned n);

#endif