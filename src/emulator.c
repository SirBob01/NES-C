#include "./emulator.h"

void create_emulator(emulator_t *emu, const char *rom_path) {
    load_rom(&emu->rom, rom_path);
    create_cpu(&emu->cpu, &emu->cpu_bus, &emu->interrupt);
    create_apu(&emu->apu, &emu->interrupt);
    create_ppu(&emu->ppu, &emu->ppu_bus, &emu->interrupt);
    create_cpu_bus(&emu->cpu_bus, &emu->rom, &emu->apu, &emu->ppu);
    create_ppu_bus(&emu->ppu_bus, &emu->rom);
    reset_interrupt(&emu->interrupt);

    // Initialize frame counter variables
    emu->cycle_accumulator = 0;
    emu->frames = 0;

    // Set the program counter
    emu->cpu.pc = read_short_cpu_bus(&emu->cpu_bus, CPU_VEC_RESET);
}

void destroy_emulator(emulator_t *emu) {
    destroy_cpu(&emu->cpu);
    destroy_apu(&emu->apu);
    destroy_ppu(&emu->ppu);
    unload_rom(&emu->rom);
}

bool update_emulator(emulator_t *emu) {
    unsigned prev_cycles = emu->cpu.cycles;
    bool cpu_state = update_cpu(&emu->cpu);
    unsigned delta_cycles = emu->cpu.cycles - prev_cycles;

    // Update peripherals
    update_apu(&emu->apu);
    for (unsigned c = 0; c < 3 * delta_cycles; c++) {
        update_ppu(&emu->ppu);
    }

    // Reset interrupts
    reset_interrupt(&emu->interrupt);

    // Update frame counter
    emu->cycle_accumulator += delta_cycles;
    if (3 * emu->cycle_accumulator >= PPU_LINEDOTS * PPU_SCANLINES) {
        emu->cycle_accumulator = 0;
        emu->frames++;
    }
    return cpu_state;
}
