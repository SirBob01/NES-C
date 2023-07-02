#ifndef AUDIO_H
#define AUDIO_H

#include <SDL.h>

#include "./buffer.h"

#define AUDIO_BUFFER_SIZE 0x400

/**
 * @brief Audio device.
 *
 */
typedef struct {
    /**
     * @brief Audio device ID.
     *
     */
    SDL_AudioDeviceID id;

    /**
     * @brief Audio device specification.
     *
     */
    SDL_AudioSpec spec;
} audio_t;

/**
 * @brief Create the audio device.
 *
 * @param audio
 * @param buffer
 */
void create_audio(audio_t *audio, buffer_t *buffer);

/**
 * @brief Destroy the audio device.
 *
 */
void destroy_audio(audio_t *device);

/**
 * @brief SDL audio callback.
 *
 * @param userdata
 * @param stream
 * @param length
 */
void play_callback_audio(void *userdata, unsigned char *stream, int length);

#endif