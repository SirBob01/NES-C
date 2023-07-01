#include "./apu.h"

apu_t *create_apu() {
    apu_t *apu = (apu_t *)malloc(sizeof(apu_t));
    apu->buffer = create_buffer(AUDIO_BUFFER_SIZE);
    apu->interrupt = NULL;
    return apu;
}

void destroy_apu(apu_t *apu) {
    destroy_buffer(apu->buffer);
    free(apu);
}

void update_apu(apu_t *apu) {
    // TODO: Implement this
}