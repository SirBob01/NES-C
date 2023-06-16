#include "./apu.h"

apu_t create_apu(cpu_t *cpu) {
    apu_t apu;
    apu.cpu = cpu;
    apu.buffer = create_buffer(AUDIO_BUFFER_SIZE);
    return apu;
}

void destroy_apu(apu_t *apu) { destroy_buffer(&apu->buffer); }

unsigned char read_register_apu(apu_t *apu, rom_t *rom, address_t address) {
    return read_byte_cpu(apu->cpu, rom, address);
}

void update_apu(apu_t *apu) {}