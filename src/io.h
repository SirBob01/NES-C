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
} io_t;

/**
 * @brief Create the IO subsystems.
 *
 * @return io_t
 */
io_t create_io(emulator_t *emu);

/**
 * @brief Destroy the IO subsystems.
 *
 * @param io
 */
void destroy_io(io_t *io);

/**
 * @brief Refresh the IO subsystems.
 *
 * @param io
 * @return true
 * @return false
 */
bool refresh_io(io_t *io);

#endif