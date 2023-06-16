#ifndef EMULATOR_H
#define EMULATOR_H

#include "./apu.h"
#include "./cpu.h"
#include "./memory.h"
#include "./rom.h"

/**
 * @brief Emulator structure holding all its subsystems.
 *
 */
typedef struct {
    rom_t rom;
    cpu_t cpu;
    apu_t apu;
} emulator_t;

/**
 * @brief Create the emulator.
 *
 * @param rom_path
 * @return emulator_t
 */
emulator_t create_emulator(const char *rom_path);

/**
 * @brief Create the emulator with a hard-coded initial program counter.
 *
 * This is useful for debugging.
 *
 * @param rom_path
 * @param pc
 * @return emulator_t
 */
emulator_t create_emulator2(const char *rom_path, address_t pc);

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