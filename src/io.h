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
 * @brief Draw the pattern tables to a display. Note that display must be big
 * enough to fit both tables, totalling 256 tiles of 8x8 pixels each (128x128
 * pixels total).
 *
 * @param display
 * @param rom
 */
void debug_pattern_tables_io(display_t *display, rom_t *rom);

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