#ifndef APU_H
#define APU_H

#include "./buffer.h"
#include "./interrupt.h"

#define AUDIO_BUFFER_SIZE 0x400

/**
 * @brief APU registers.
 *
 */
typedef struct {
    /**
     * @brief Pulse 1 channel registers.
     *
     */
    unsigned char pulse1[4];

    /**
     * @brief Pulse 2 channel registers.
     *
     */
    unsigned char pulse2[4];

    /**
     * @brief Triangle channel registers.
     *
     */
    unsigned char triangle[4];

    /**
     * @brief Noise channel registers.
     *
     */
    unsigned char noise[4];

    /**
     * @brief DMC channel registers.
     *
     */
    unsigned char dmc[4];
} apu_channel_registers_t;

/**
 * @brief The Audio Processing Unit is responsible for generating sound.
 *
 */
typedef struct {
    /**
     * @brief Audio output buffer.
     *
     */
    buffer_t buffer;

    /**
     * @brief Internal channel registers.
     *
     */
    apu_channel_registers_t channel_registers;

    /**
     * @brief Status/control register.
     *
     */
    unsigned char status;

    /**
     * @brief Frame counter register.
     *
     */
    unsigned char frame_counter;

    /**
     * @brief Number of cycles.
     *
     */
    unsigned long cycles;

    /**
     * @brief Pointer to the interrupt state.
     *
     */
    interrupt_t *interrupt;
} apu_t;

/**
 * @brief Create the APU.
 *
 * @param apu
 * @param interrupt
 */
void create_apu(apu_t *apu, interrupt_t *interrupt);

/**
 * @brief Destroy the APU.
 *
 * @param apu
 */
void destroy_apu(apu_t *apu);

/**
 * @brief Update the APU.
 *
 * @param apu
 */
void update_apu(apu_t *apu);

#endif