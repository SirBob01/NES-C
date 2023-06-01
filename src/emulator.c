#include "./emulator.h"

emulator_t create_emulator(const char *rom_path) {
    emulator_t emu;
    emu.rom = load_rom(rom_path);
    emu.cpu = create_cpu();
    if (emu.rom.data.buffer == NULL) {
        return emu;
    }

    // Set the program counter
    unsigned char *rv = get_memory_cpu(&emu.cpu, &emu.rom, CPU_VEC_RESET);
    emu.cpu.registers.pc = rv[0] | (rv[1] << 8);

    return emu;
}

emulator_t create_emulator2(const char *rom_path, unsigned short pc) {
    emulator_t emu;
    emu.rom = load_rom(rom_path);
    emu.cpu = create_cpu();
    if (emu.rom.data.buffer == NULL) {
        return emu;
    }

    // Set the program counter
    emu.cpu.registers.pc = pc;

    return emu;
}

void destroy_emulator(emulator_t *emulator) {
    destroy_cpu(&emulator->cpu);
    unload_rom(&emulator->rom);
}