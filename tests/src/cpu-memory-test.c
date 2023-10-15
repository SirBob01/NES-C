#include <stdio.h>

#include "./ctest.h"

#include "../../src/emulator.h"

int tests_run = 0;

static char *test_mirror_ram() {
    // RAM
    mu_assert("RAM (L)", mirror_cpu_bus(0x0000, 0x2000) == 0);
    mu_assert("RAM (M)", mirror_cpu_bus(0x0050, 0x2000) == 0x050);
    mu_assert("RAM (H)", mirror_cpu_bus(0x07ff, 0x2000) == 0x07ff);

    // Mirror 0
    mu_assert("RAM Mirror 0 (L)", mirror_cpu_bus(0x0800, 0x2000) == 0);
    mu_assert("RAM Mirror 0 (M)", mirror_cpu_bus(0x0900, 0x2000) == 0x0100);
    mu_assert("RAM Mirror 0 (H)", mirror_cpu_bus(0x0fff, 0x2000) == 0x07ff);

    // Mirror 1
    mu_assert("RAM Mirror 1 (L)", mirror_cpu_bus(0x1000, 0x2000) == 0);
    mu_assert("RAM Mirror 1 (M)", mirror_cpu_bus(0x1100, 0x2000) == 0x0100);
    mu_assert("RAM Mirror 1 (H)", mirror_cpu_bus(0x17ff, 0x2000) == 0x07ff);

    // Mirror 2
    mu_assert("RAM Mirror 2 (L)", mirror_cpu_bus(0x1800, 0x2000) == 0);
    mu_assert("RAM Mirror 2 (M)", mirror_cpu_bus(0x1900, 0x2000) == 0x0100);
    mu_assert("RAM Mirror 2 (H)", mirror_cpu_bus(0x1fff, 0x2000) == 0x07ff);
    return 0;
}

static char *test_mirror_ppu() {
    // PPU
    mu_assert("PPU (L)", mirror_cpu_bus(0x2000, 0x2000) == 0x2000);
    mu_assert("PPU (M)", mirror_cpu_bus(0x2004, 0x2000) == 0x2004);
    mu_assert("PPU (H)", mirror_cpu_bus(0x2007, 0x2000) == 0x2007);

    // First Mirror
    mu_assert("PPU Mirror 0 (L)", mirror_cpu_bus(0x2008, 0x2000) == 0x2000);
    mu_assert("PPU Mirror 0 (M)", mirror_cpu_bus(0x200c, 0x2000) == 0x2004);
    mu_assert("PPU Mirror 0 (H)", mirror_cpu_bus(0x200f, 0x2000) == 0x2007);

    // Last Mirror
    mu_assert("PPU Mirror last (L)", mirror_cpu_bus(0x3ff8, 0x2000) == 0x2000);
    mu_assert("PPU Mirror last (M)", mirror_cpu_bus(0x3ffc, 0x2000) == 0x2004);
    mu_assert("PPU Mirror last (H)", mirror_cpu_bus(0x3fff, 0x2000) == 0x2007);
    return 0;
}

static char *test_mirror_apu() {
    mu_assert("APU I/O (L)", mirror_cpu_bus(0x4000, 0x2000) == 0x4000);
    mu_assert("APU I/O (M)", mirror_cpu_bus(0x4017, 0x2000) == 0x4017);
    mu_assert("APU I/O (H)", mirror_cpu_bus(0x4019, 0x2000) == 0x4019);
    return 0;
}

static char *test_mapper0() {
    emulator_t emu;
    create_emulator(&emu, "../roms/nestest/nestest.nes");

    unsigned char pcl = read_cpu_bus(&emu.cpu_bus, CPU_VEC_RESET);
    unsigned char pch = read_cpu_bus(&emu.cpu_bus, CPU_VEC_RESET + 1);
    address_t pc = (pch << 8) | pcl;
    mu_assert("MAPPER 0 RESET VECTOR", pc == 0xc004);

    destroy_emulator(&emu);
    return 0;
}

static char *test_stack() {
    emulator_t emu;
    create_emulator(&emu, "../roms/nestest/nestest.nes");
    mu_assert("Stack address start", emu.cpu.s == 0xfd);

    push_stack_cpu(&emu.cpu, 0x3);
    mu_assert("Stack address after push", emu.cpu.s == 0xfc);

    mu_assert("Stack pop top byte", pop_stack_cpu(&emu.cpu) == 0x3);
    mu_assert("Stack address after pop", emu.cpu.s == 0xfd);

    destroy_emulator(&emu);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_mirror_ram);
    mu_run_test(test_mirror_ppu);
    mu_run_test(test_mirror_apu);
    mu_run_test(test_mapper0);
    mu_run_test(test_stack);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("FAILED... %s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Number of tests run: %d\n", tests_run);

    return result != 0;
}