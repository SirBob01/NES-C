#ifndef IO_H
#define IO_H

#include "./audio.h"
#include "./display.h"
#include "./emulator.h"
#include "./input.h"

/**
 * @brief Device subsystems for simulating I/O hardware.
 *
 */
typedef struct {
    /**
     * @brief Logical emulator.
     *
     */
    emulator_t *emu;

    /**
     * @brief Main display.
     *
     */
    display_t display;

    /**
     * @brief Sound system.
     *
     */
    audio_t audio;

    /**
     * @brief Input event queue.
     *
     */
    input_t input;

    /**
     * @brief Pattern table display (debug only).
     *
     */
    display_t pattern_table;

    /**
     * @brief Nametable display (debug only).
     *
     */
    display_t nametables;
} io_t;

/**
 * @brief Create the I/O interfaces.
 *
 * @param io
 * @param emu
 */
void create_io(io_t *io, emulator_t *emu);

/**
 * @brief Destroy the I/O interfaces.
 *
 * @param io
 */
void destroy_io(io_t *io);

/**
 * @brief Enable or disable debug mode for the I/O interfaces.
 *
 * @param io
 * @param debug
 */
void set_debug_io(io_t *io, bool debug);

/**
 * @brief Check if the I/O interfaces are in debug mode.
 *
 * @param io
 * @return true
 * @return false
 */
bool is_debug_io(io_t *io);

/**
 * @brief Refresh the I/O interfaces.
 *
 * @param io
 * @param emu
 * @return true
 * @return false
 */
bool refresh_io(io_t *io, emulator_t *emu);

#endif