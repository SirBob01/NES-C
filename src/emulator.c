#include "./emulator.h"
#include "ppu.h"

emulator_t *create_emulator(const char *rom_path) {
    emulator_t *emu = (emulator_t *)malloc(sizeof(emulator_t));
    emu->rom = load_rom(rom_path);
    emu->ppu = create_ppu(emu->rom);
    emu->apu = create_apu();
    emu->cpu = create_cpu(emu->rom, emu->apu, emu->ppu);

    // Initialize frame counter variables
    emu->cycle_accumulator = 0;
    emu->frames = 0;

    // Set the program counter
    unsigned char *rv = get_memory_cpu(emu->cpu, CPU_VEC_RESET);
    emu->cpu->pc = rv[0] | (rv[1] << 8);

    return emu;
}

void destroy_emulator(emulator_t *emu) {
    destroy_cpu(emu->cpu);
    destroy_apu(emu->apu);
    destroy_ppu(emu->ppu);
    unload_rom(emu->rom);
    free(emu);
}

bool update_emulator(emulator_t *emu) {
    unsigned prev_cycles = emu->cpu->cycles;
    bool cpu_state = update_cpu(emu->cpu);
    unsigned delta_cycles = emu->cpu->cycles - prev_cycles;

    // Update peripherals
    update_apu(emu->apu);
    for (unsigned c = 0; c < 3 * delta_cycles; c++) {
        update_ppu(emu->ppu);
    }

    // Update frame counter
    emu->cycle_accumulator += delta_cycles;
    if (3 * emu->cycle_accumulator >= PPU_LINEDOTS * PPU_SCANLINES) {
        emu->cycle_accumulator = 0;
        emu->frames++;
    }
    return cpu_state;
}
