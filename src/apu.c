#include "./apu.h"

void create_apu(apu_t *apu, interrupt_t *interrupt) {
    apu->interrupt = interrupt;
    apu->cycles = 0;
    create_buffer(&apu->buffer, AUDIO_BUFFER_SIZE);
}

void destroy_apu(apu_t *apu) { destroy_buffer(&apu->buffer); }

void update_apu(apu_t *apu) {
    // TODO: Implement this
    apu->cycles++;
}