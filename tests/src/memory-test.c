#include <stdio.h>

#include "./ctest.h"

#include "../../src/cpu.h"
#include "../../src/rom.h"

int tests_run = 0;

static char *test_mirror_ram() {
    // RAM
    mu_assert("RAM (S)", mirror_address_cpu(0x0000) == 0);
    mu_assert("RAM (M)", mirror_address_cpu(0x0050) == 0x050);
    mu_assert("RAM (E)", mirror_address_cpu(0x07ff) == 0x07ff);

    // Mirror 0
    mu_assert("RAM Mirror 0 (S)", mirror_address_cpu(0x0800) == 0);
    mu_assert("RAM Mirror 0 (M)", mirror_address_cpu(0x0900) == 0x0100);
    mu_assert("RAM Mirror 0 (E)", mirror_address_cpu(0x0fff) == 0x07ff);

    // Mirror 1
    mu_assert("RAM Mirror 1 (S)", mirror_address_cpu(0x1000) == 0);
    mu_assert("RAM Mirror 1 (M)", mirror_address_cpu(0x1100) == 0x0100);
    mu_assert("RAM Mirror 1 (E)", mirror_address_cpu(0x17ff) == 0x07ff);

    // Mirror 2
    mu_assert("RAM Mirror 2 (S)", mirror_address_cpu(0x1800) == 0);
    mu_assert("RAM Mirror 2 (M)", mirror_address_cpu(0x1900) == 0x0100);
    mu_assert("RAM Mirror 2 (E)", mirror_address_cpu(0x1fff) == 0x07ff);
    return 0;
}

static char *test_mirror_ppu() {
    // PPU
    mu_assert("PPU (S)", mirror_address_cpu(0x2000) == 0x2000);
    mu_assert("PPU (M)", mirror_address_cpu(0x2004) == 0x2004);
    mu_assert("PPU (E)", mirror_address_cpu(0x2007) == 0x2007);

    // First Mirror
    mu_assert("PPU Mirror 0 (S)", mirror_address_cpu(0x2008) == 0x2000);
    mu_assert("PPU Mirror 0 (M)", mirror_address_cpu(0x200c) == 0x2004);
    mu_assert("PPU Mirror 0 (E)", mirror_address_cpu(0x200f) == 0x2007);

    // Last Mirror
    mu_assert("PPU Mirror last (S)", mirror_address_cpu(0x3ff8) == 0x2000);
    mu_assert("PPU Mirror last (M)", mirror_address_cpu(0x3ffc) == 0x2004);
    mu_assert("PPU Mirror last (E)", mirror_address_cpu(0x3fff) == 0x2007);
    return 0;
}

static char *test_mirror_apu() {
    mu_assert("APU I/O (S)", mirror_address_cpu(0x4000) == 0x4000);
    mu_assert("APU I/O (M)", mirror_address_cpu(0x4017) == 0x4017);
    mu_assert("APU I/O (E)", mirror_address_cpu(0x4019) == 0x4019);
    return 0;
}

static char *test_mapper0() {
    rom_t rom = load_rom("../roms/nestest.nes");
    cpu_t cpu = create_cpu();

    unsigned char pc0 = read_cpu(&cpu, &rom, CPU_VEC_RESET);
    unsigned char pc1 = read_cpu(&cpu, &rom, CPU_VEC_RESET + 1);
    mu_assert("MAPPER 0 RESET VECTOR", (pc0 | (pc1 << 8)) == 0xc004);

    free_rom(&rom);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_mirror_ram);
    mu_run_test(test_mirror_ppu);
    mu_run_test(test_mirror_apu);
    mu_run_test(test_mapper0);
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