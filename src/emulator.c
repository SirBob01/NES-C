#include "./emulator.h"

emulator_t create_emulator(const char *rom_path) {
    emulator_t emu;
    emu.rom = load_rom(rom_path);
    emu.cpu = create_cpu();

    // Set the program counter
    unsigned char *rv = get_memory_cpu(&emu.cpu, &emu.rom, CPU_VEC_RESET);
    emu.cpu.registers.pc = rv[0] | (rv[1] << 8);

    return emu;
}

emulator_t create_emulator2(const char *rom_path, unsigned short pc) {
    emulator_t emu;
    emu.rom = load_rom(rom_path);
    emu.cpu = create_cpu();

    // Set the program counter
    emu.cpu.registers.pc = pc;

    return emu;
}

bool update_emulator(emulator_t *emu) {
    unsigned short pc = emu->cpu.registers.pc++;
    unsigned char opcode = read_cpu(&emu->cpu, &emu->rom, pc);
    switch (opcode) {
    default:
        fprintf(stderr, "Error: unknown opcode 0x%02x\n", opcode);
        exit(1);
        break;
    }
    return true;
}

void destroy_emulator(emulator_t *emu) {
    destroy_cpu(&emu->cpu);
    unload_rom(&emu->rom);
}