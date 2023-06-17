#include "./apu.h"

apu_t *create_apu(cpu_t *cpu, rom_t *rom) {
    apu_t *apu = (apu_t *)malloc(sizeof(apu_t));
    apu->buffer = create_buffer(AUDIO_BUFFER_SIZE);
    apu->cpu = cpu;
    apu->rom = rom;
    return apu;
}

void destroy_apu(apu_t *apu) {
    destroy_buffer(apu->buffer);
    free(apu);
}

unsigned char read_register_apu(apu_t *apu, address_t address) {
    return read_byte_cpu(apu->cpu, address);
}

void update_apu(apu_t *apu) {}