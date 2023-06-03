#include "./emulator.h"

emulator_t create_emulator(const char *rom_path) {
    emulator_t emu;
    emu.rom = load_rom(rom_path);
    emu.cpu = create_cpu();

    // Set the program counter
    unsigned char *rv = get_memory_cpu(&emu.cpu, &emu.rom, CPU_VEC_RESET);
    emu.cpu.pc = rv[0] | (rv[1] << 8);

    return emu;
}

emulator_t create_emulator2(const char *rom_path, address_t pc) {
    emulator_t emu;
    emu.rom = load_rom(rom_path);
    emu.cpu = create_cpu();

    // Set the program counter
    emu.cpu.pc = pc;

    return emu;
}

void destroy_emulator(emulator_t *emu) {
    destroy_cpu(&emu->cpu);
    unload_rom(&emu->rom);
}

bool update_emulator(emulator_t *emu) {
    update_cpu(&emu->cpu, &emu->rom);
    return true;
}
