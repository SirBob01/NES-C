#ifndef EMULATOR_H
#define EMULATOR_H

#include "./apu.h"
#include "./cpu.h"
#include "./cpu_bus.h"
#include "./memory.h"
#include "./ppu.h"
#include "./ppu_bus.h"
#include "./rom.h"

/**
 * @brief Emulator structure holding all its subsystems.
 *
 */
typedef struct {
    /**
     * @brief Cartridge read-only memory.
     *
     */
    rom_t rom;

    /**
     * @brief Central procesing unit.
     *
     */
    cpu_t cpu;

    /**
     * @brief Audio processing unit.
     *
     */
    apu_t apu;

    /**
     * @brief Picture processing unit.
     *
     */
    ppu_t ppu;

    /**
     * @brief CPU bus.
     *
     */
    cpu_bus_t cpu_bus;

    /**
     * @brief PPU bus.
     *
     */
    ppu_bus_t ppu_bus;

    /**
     * @brief Interrupt state.
     *
     */
    interrupt_t interrupt;

    /**
     * @brief Accumulator for frame-counting.
     *
     */
    unsigned cycle_accumulator;

    /**
     * @brief Frame counter.
     *
     */
    unsigned frames;
} emulator_t;

/**
 * @brief Create an emulator.
 *
 * @param emu
 * @param rom_path
 */
void create_emulator(emulator_t *emu, const char *rom_path);

/**
 * @brief Free all resources held by the emulator.
 *
 * @param emu
 */
void destroy_emulator(emulator_t *emu);

/**
 * @brief Update the emulator.
 *
 * @param emu
 */
void update_emulator(emulator_t *emu);

#endif