#include "./apu.h"

void create_apu(apu_t *apu, interrupt_t *interrupt) {
    apu->interrupt = interrupt;
    apu->cycles = 0;
    create_buffer(&apu->buffer, AUDIO_BUFFER_SIZE);
}

void destroy_apu(apu_t *apu) { destroy_buffer(&apu->buffer); }

unsigned char read_status_apu(apu_t *apu) { return apu->status; }

void write_status_apu(apu_t *apu, unsigned char value) {
    // Clear DMC interrupt flag
    apu->status = value;
    apu->channel_registers.dmc[0] &= ~APU_DMC_INTERRUPT;
}

void update_apu(apu_t *apu) {
    // TODO: Implement this
    apu->cycles++;
}