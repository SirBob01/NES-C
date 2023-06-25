#ifndef EMULATOR_H
#define EMULATOR_H

#include "./apu.h"
#include "./cpu.h"
#include "./memory.h"
#include "./ppu.h"
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
    rom_t *rom;

    /**
     * @brief Picture processing unit.
     *
     */
    ppu_t *ppu;

    /**
     * @brief Audio processing unit.
     *
     */
    apu_t *apu;

    /**
     * @brief Central procesing unit.
     *
     */
    cpu_t *cpu;

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
 * @brief Create a emulator.
 *
 * @param rom_path
 * @return emulator_t*
 */
emulator_t *create_emulator(const char *rom_path);

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
 * @return true
 * @return false
 */
bool update_emulator(emulator_t *emu);

#endif