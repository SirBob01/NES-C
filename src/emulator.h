#ifndef EMULATOR_H
#define EMULATOR_H

#include "./cpu.h"
#include "./memory.h"
#include "./opcode.h"
#include "./rom.h"

/**
 * @brief Emulator structure holding all its subsystems.
 *
 */
typedef struct {
    rom_t rom;
    cpu_t cpu;
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
emulator_t create_emulator2(const char *rom_path, unsigned short pc);

/**
 * @brief Free all resources held by the emulator.
 *
 * @param emu
 */
void destroy_emulator(emulator_t *emu);

/**
 * @brief Read a byte from the CPU's memory.
 *
 * @param emu
 * @param address
 * @return unsigned char
 */
unsigned char read_cpu_byte(emulator_t *emu, unsigned short address);

/**
 * @brief Read a short from the CPU's memory.
 *
 * @param emu
 * @param address
 * @return unsigned short
 */
unsigned short read_cpu_short(emulator_t *emu, unsigned short address);

/**
 * @brief Write a byte to the CPU's memory.
 *
 * @param emu
 * @param address
 * @param value
 */
void write_cpu_byte(emulator_t *emu,
                    unsigned short address,
                    unsigned char value);

/**
 * @brief Write a short to the CPU's memory.
 *
 * @param emu
 * @param address
 * @param value
 */
void write_cpu_short(emulator_t *emu,
                     unsigned short address,
                     unsigned short value);

/**
 * @brief Run the emulator.
 *
 * @param emu
 * @return true
 * @return false
 */
bool update_emulator(emulator_t *emu);

#endif