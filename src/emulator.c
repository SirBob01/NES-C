#include "./emulator.h"

emulator_t *create_emulator(const char *rom_path) {
    emulator_t *emu = (emulator_t *)malloc(sizeof(emulator_t));
    emu->rom = load_rom(rom_path);
    emu->cpu = create_cpu(emu->rom);
    emu->apu = create_apu(emu->cpu);

    // Set the program counter
    unsigned char *rv = get_memory_cpu(emu->cpu, CPU_VEC_RESET);
    emu->cpu->pc = rv[0] | (rv[1] << 8);

    return emu;
}

void destroy_emulator(emulator_t *emu) {
    destroy_apu(emu->apu);
    destroy_cpu(emu->cpu);
    unload_rom(emu->rom);
    free(emu);
}

bool update_emulator(emulator_t *emu) {
    bool cpu_state = update_cpu(emu->cpu);
    update_apu(emu->apu);

    return cpu_state;
}
