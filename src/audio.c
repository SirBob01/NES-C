#include "./audio.h"

void create_audio(audio_t *audio, buffer_t *buffer) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);

    desired_spec.channels = 1;
    desired_spec.freq = 44100;
    desired_spec.format = SDL_AUDIO_F32LSB;
    desired_spec.samples = AUDIO_BUFFER_SIZE;
    desired_spec.userdata = buffer;
    desired_spec.callback = play_callback_audio;

    audio->id = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &audio->spec, 0);
    if (!audio->id) {
        fprintf(stderr,
                "Error: Failed to open audio device (%s)\n",
                SDL_GetError());
        exit(1);
    }

    int result = SDL_PlayAudioDevice(audio->id);
    if (result < 0) {
        fprintf(stderr,
                "Error: Failed to play audio device (%s)\n",
                SDL_GetError());
        exit(1);
    }
}

void destroy_audio(audio_t *device) { SDL_CloseAudioDevice(device->id); }

void play_callback_audio(void *userdata, unsigned char *stream, int length) {
    buffer_t *buffer = (buffer_t *)userdata;
    unsigned read = read_buffer(buffer, stream, length);
    memset(stream + read, 0, length - read);
}