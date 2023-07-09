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
    emu->cpu.registers.pc = read_short_cpu_bus(&emu->cpu_bus, CPU_VEC_RESET);
}

void destroy_emulator(emulator_t *emu) {
    destroy_cpu(&emu->cpu);
    destroy_apu(&emu->apu);
    destroy_ppu(&emu->ppu);
    unload_rom(&emu->rom);
}

void update_emulator(emulator_t *emu) {
    // Update the CPU
    update_cpu(&emu->cpu);

    // Update APU every other cycle
    if ((emu->cpu.cycles & 1) == 0) {
        update_apu(&emu->apu);
    }

    // Update the PPU 3 times a cycle
    for (unsigned c = 0; c < 3; c++) {
        update_ppu(&emu->ppu);
    }

    // Update frame counter
    emu->cycle_accumulator++;
    if (3 * emu->cycle_accumulator >= PPU_LINEDOTS * PPU_SCANLINES) {
        emu->cycle_accumulator = 0;
        emu->frames++;
    }
}
