#include <stdio.h>

#include "./ctest.h"

#include "../../src/cpu.h"
#include "../../src/rom.h"

int tests_run = 0;

static char *test_map_ram() {
    rom_t rom = load_rom("../roms/nestest.nes");

    // RAM
    mu_assert("RAM (S)", map_address_cpu(&rom, 0x0000) == 0);
    mu_assert("RAM (M)", map_address_cpu(&rom, 0x0050) == 0x050);
    mu_assert("RAM (E)", map_address_cpu(&rom, 0x07ff) == 0x07ff);

    // Mirror 0
    mu_assert("RAM Mirror 0 (S)", map_address_cpu(&rom, 0x0800) == 0);
    mu_assert("RAM Mirror 0 (M)", map_address_cpu(&rom, 0x0900) == 0x0100);
    mu_assert("RAM Mirror 0 (E)", map_address_cpu(&rom, 0x0fff) == 0x07ff);

    // Mirror 1
    mu_assert("RAM Mirror 1 (S)", map_address_cpu(&rom, 0x1000) == 0);
    mu_assert("RAM Mirror 1 (M)", map_address_cpu(&rom, 0x1100) == 0x0100);
    mu_assert("RAM Mirror 1 (E)", map_address_cpu(&rom, 0x17ff) == 0x07ff);

    // Mirror 2
    mu_assert("RAM Mirror 2 (S)", map_address_cpu(&rom, 0x1800) == 0);
    mu_assert("RAM Mirror 2 (M)", map_address_cpu(&rom, 0x1900) == 0x0100);
    mu_assert("RAM Mirror 2 (E)", map_address_cpu(&rom, 0x1fff) == 0x07ff);

    free_rom(&rom);
    return 0;
}

static char *test_map_ppu() {
    rom_t rom = load_rom("../roms/nestest.nes");

    // PPU
    mu_assert("PPU (S)", map_address_cpu(&rom, 0x2000) == 0x2000);
    mu_assert("PPU (M)", map_address_cpu(&rom, 0x2004) == 0x2004);
    mu_assert("PPU (E)", map_address_cpu(&rom, 0x2007) == 0x2007);

    // First Mirror
    mu_assert("PPU Mirror 0 (S)", map_address_cpu(&rom, 0x2008) == 0x2000);
    mu_assert("PPU Mirror 0 (M)", map_address_cpu(&rom, 0x200c) == 0x2004);
    mu_assert("PPU Mirror 0 (E)", map_address_cpu(&rom, 0x200f) == 0x2007);

    // Last Mirror
    mu_assert("PPU Mirror last (S)", map_address_cpu(&rom, 0x3ff8) == 0x2000);
    mu_assert("PPU Mirror last (M)", map_address_cpu(&rom, 0x3ffc) == 0x2004);
    mu_assert("PPU Mirror last (E)", map_address_cpu(&rom, 0x3fff) == 0x2007);

    free_rom(&rom);
    return 0;
}

static char *test_map_apu() {
    rom_t rom = load_rom("../roms/nestest.nes");

    mu_assert("APU I/O (S)", map_address_cpu(&rom, 0x4000) == 0x4000);
    mu_assert("APU I/O (M)", map_address_cpu(&rom, 0x4017) == 0x4017);
    mu_assert("APU I/O (E)", map_address_cpu(&rom, 0x4019) == 0x4019);

    free_rom(&rom);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_map_ram);
    mu_run_test(test_map_ppu);
    mu_run_test(test_map_apu);
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