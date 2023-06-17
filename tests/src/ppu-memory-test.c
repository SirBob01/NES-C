#include <stdio.h>

#include "./ctest.h"

#include "../../src/emulator.h"
#include "../../src/mappers/nrom.h"

int tests_run = 0;

static char *test_mirror_nametables_hor() {
    mu_assert("Mirror Nametables 0 H (L)",
              mirror_address_ppu(0x2000, MIRROR_HORIZONTAL) == 0x2000);
    mu_assert("Mirror Nametables 0 H (H)",
              mirror_address_ppu(0x23ff, MIRROR_HORIZONTAL) == 0x23ff);

    mu_assert("Mirror Nametables 1 H (L)",
              mirror_address_ppu(0x2400, MIRROR_HORIZONTAL) == 0x2000);
    mu_assert("Mirror Nametables 1 H (H)",
              mirror_address_ppu(0x27ff, MIRROR_HORIZONTAL) == 0x23ff);

    mu_assert("Mirror Nametables 2 H (L)",
              mirror_address_ppu(0x2800, MIRROR_HORIZONTAL) == 0x2800);
    mu_assert("Mirror Nametables 2 H (H)",
              mirror_address_ppu(0x2bff, MIRROR_HORIZONTAL) == 0x2bff);

    mu_assert("Mirror Nametables 3 H (L)",
              mirror_address_ppu(0x2c00, MIRROR_HORIZONTAL) == 0x2800);
    mu_assert("Mirror Nametables 3 H (H)",
              mirror_address_ppu(0x2fff, MIRROR_HORIZONTAL) == 0x2bff);

    return 0;
}

static char *test_mirror_nametables_ver() {
    mu_assert("Mirror Nametables 0 V (L)",
              mirror_address_ppu(0x2000, MIRROR_VERTICAL) == 0x2000);
    mu_assert("Mirror Nametables 0 V (H)",
              mirror_address_ppu(0x23ff, MIRROR_VERTICAL) == 0x23ff);

    mu_assert("Mirror Nametables 1 V (L)",
              mirror_address_ppu(0x2400, MIRROR_VERTICAL) == 0x2400);
    mu_assert("Mirror Nametables 1 V (H)",
              mirror_address_ppu(0x27ff, MIRROR_VERTICAL) == 0x27ff);

    mu_assert("Mirror Nametables 2 V (L)",
              mirror_address_ppu(0x2800, MIRROR_VERTICAL) == 0x2000);
    mu_assert("Mirror Nametables 2 V (H)",
              mirror_address_ppu(0x2bff, MIRROR_VERTICAL) == 0x23ff);

    mu_assert("Mirror Nametables 3 V (L)",
              mirror_address_ppu(0x2c00, MIRROR_VERTICAL) == 0x2400);
    mu_assert("Mirror Nametables 3 V (H)",
              mirror_address_ppu(0x2fff, MIRROR_VERTICAL) == 0x27ff);

    return 0;
}

static char *test_mirror_palettes() {
    // Palette
    mu_assert("Palette (L)",
              mirror_address_ppu(0x3f00, MIRROR_HORIZONTAL) == 0x3f00);
    mu_assert("Palette (M)",
              mirror_address_ppu(0x3f0f, MIRROR_HORIZONTAL) == 0x3f0f);
    mu_assert("Palette (H)",
              mirror_address_ppu(0x3f1f, MIRROR_HORIZONTAL) == 0x3f1f);

    // First Mirror
    mu_assert("Palette Mirror 0 (L)",
              mirror_address_ppu(0x3f20, MIRROR_HORIZONTAL) == 0x3f00);
    mu_assert("Palette Mirror 0 (M)",
              mirror_address_ppu(0x3f2f, MIRROR_HORIZONTAL) == 0x3f0f);
    mu_assert("Palette Mirror 0 (H)",
              mirror_address_ppu(0x3f3f, MIRROR_HORIZONTAL) == 0x3f1f);

    // Last Mirror
    mu_assert("Palette Mirror Last (L)",
              mirror_address_ppu(0x3fe0, MIRROR_HORIZONTAL) == 0x3f00);
    mu_assert("Palette Mirror Last (M)",
              mirror_address_ppu(0x3fef, MIRROR_HORIZONTAL) == 0x3f0f);
    mu_assert("Palette Mirror Last (H)",
              mirror_address_ppu(0x3fff, MIRROR_HORIZONTAL) == 0x3f1f);
    return 0;
}

static char *test_mapper0() {
    emulator_t *emu = create_emulator("../roms/nestest.nes");

    mu_assert("CHR_ROM[0]", *nrom_ppu(emu->ppu, 0x20) == 0x80);
    mu_assert("CHR_ROM[1]", *nrom_ppu(emu->ppu, 0x21) == 0x80);
    mu_assert("CHR_ROM[2]", *nrom_ppu(emu->ppu, 0x22) == 0xff);
    mu_assert("CHR_ROM[2]", *nrom_ppu(emu->ppu, 0x23) == 0x80);

    destroy_emulator(emu);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_mirror_nametables_hor);
    mu_run_test(test_mirror_nametables_ver);
    mu_run_test(test_mirror_palettes);
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