#ifndef APU_H
#define APU_H

#include "./buffer.h"
#include "./cpu.h"

#define AUDIO_BUFFER_SIZE   0x400

#define APU_PULSE1_ENVELOPE 0x4000
#define APU_PULSE1_SWEEP    0x4001
#define APU_PULSE1_TIMER    0x4002
#define APU_PULSE1_LENGTH   0x4003
#define APU_PULSE2_ENVELOPE 0x4004
#define APU_PULSE2_SWEEP    0x4005
#define APU_PULSE2_TIMER    0x4006
#define APU_PULSE2_LENGTH   0x4007
#define APU_TRIANGLE_LINEAR 0x4008
#define APU_TRIANGLE_TIMER  0x400A
#define APU_TRIANGLE_LENGTH 0x400B
#define APU_NOISE_ENVELOPE  0x400C
#define APU_NOISE_TIMER     0x400E
#define APU_NOISE_LENGTH    0x400F
#define APU_DMC_FLAGS       0x4010
#define APU_DMC_TIMER       0x4011
#define APU_DMC_ADDRESS     0x4012
#define APU_DMC_LENGTH      0x4013
#define APU_STATUS          0x4015
#define APU_FRAME_COUNTER   0x4017

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
     * @brief Pointer to the CPU.
     *
     */
    cpu_t *cpu;
} apu_t;

/**
 * @brief Create the APU.
 *
 * @param cpu
 * @return apu_t*
 */
apu_t *create_apu(cpu_t *cpu);

/**
 * @brief Destroy the APU.
 *
 * @param apu
 */
void destroy_apu(apu_t *apu);

/**
 * @brief Read a register value of the APU.
 *
 * @param apu
 * @param address
 * @return unsigned char
 */
unsigned char read_register_apu(apu_t *apu, address_t address);

/**
 * @brief Update the APU.
 *
 * @param apu
 */
void update_apu(apu_t *apu);

#endif