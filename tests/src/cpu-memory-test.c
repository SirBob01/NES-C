#include <stdio.h>

#include "./ctest.h"

#include "../../src/emulator.h"

int tests_run = 0;

char message[1024] = {0};

static char *test_mirror_ram() {
    for (address_t addr = 0; addr < 0x2000; addr++) {
        address_t received = mirror_cpu_bus(addr);
        address_t expected = addr % 0x800;
        snprintf(message, sizeof(message), "RAM Mirror (0x%04x)", addr);
        mu_assert(message, received == expected);
    }
    return 0;
}

static char *test_mirror_ppu() {
    for (address_t addr = 0x2000; addr < 0x4000; addr++) {
        address_t received = mirror_cpu_bus(addr);
        address_t expected = 0x2000 + (addr % 0x08);
        snprintf(message, sizeof(message), "PPU Mirror (0x%04x)", addr);
        mu_assert(message, received == expected);
    }
    return 0;
}

static char *test_mirror_apu() {
    for (address_t addr = 0x4000; addr < 0x4020; addr++) {
        address_t received = mirror_cpu_bus(addr);
        snprintf(message, sizeof(message), "APU I/O (0x%04x)", addr);
        mu_assert(message, received == addr);
    }
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