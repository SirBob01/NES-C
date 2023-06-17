#ifndef APU_H
#define APU_H

#include "./buffer.h"

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
    unsigned char triangle[3];

    /**
     * @brief Noise channel registers.
     *
     */
    unsigned char noise[3];

    /**
     * @brief DMC channel registers.
     *
     */
    unsigned char dmc[4];

    /**
     * @brief Status/control register.
     *
     */
    unsigned char status_control;

    /**
     * @brief Frame counter register.
     *
     */
    unsigned char frame_counter;
} apu_registers_t;

/**
 * @brief The Audio Processing Unit is responsible for generating sound.
 *
 */
typedef struct {
    /**
     * @brief Audio output buffer.
     *
     */
    buffer_t *buffer;

    /**
     * @brief Internal registers.
     *
     */
    apu_registers_t registers;
} apu_t;

/**
 * @brief Create the APU.
 *
 * @return apu_t*
 */
apu_t *create_apu();

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