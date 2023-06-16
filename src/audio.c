#include "./audio.h"

audio_t create_audio(buffer_t *buffer) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    audio_t device;
    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);

    desired_spec.channels = 1;
    desired_spec.freq = 44100;
    desired_spec.format = SDL_AUDIO_F32LSB;
    desired_spec.samples = AUDIO_BUFFER_SIZE;
    desired_spec.userdata = buffer;
    desired_spec.callback = play_callback_audio;

    device.id = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &device.spec, 0);
    if (!device.id) {
        fprintf(stderr,
                "Error: Failed to open audio device (%s)\n",
                SDL_GetError());
        exit(1);
    }

    int result = SDL_PlayAudioDevice(device.id);
    if (result < 0) {
        fprintf(stderr,
                "Error: Failed to play audio device (%s)\n",
                SDL_GetError());
        exit(1);
    }
    return device;
}

void destroy_audio(audio_t *device) { SDL_CloseAudioDevice(device->id); }

void play_callback_audio(void *userdata, unsigned char *stream, int length) {
    buffer_t *buffer = (buffer_t *)userdata;
    unsigned read = read_buffer(buffer, stream, length);
    memset(stream + read, 0, length - read);
}