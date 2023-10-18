#include <stdio.h>

#include "./ctest.h"

#include "../../src/emulator.h"

int tests_run = 0;

char message[1024] = {0};

static char *test_mirror_nametables_hor() {
    for (unsigned addr = 0x2000; addr < 0x4000; addr++) {
        address_t received = mirror_address_ppu_bus(addr, MIRROR_HORIZONTAL);
        address_t expected = addr;
        if (expected >= 0x3000) {
            expected -= 0x1000;
        }
        if (expected < PPU_MAP_NAMETABLE_2 && expected >= PPU_MAP_NAMETABLE_1) {
            expected -= 0x400;
        }
        if (expected >= PPU_MAP_NAMETABLE_3) {
            expected -= 0x400;
        }
        snprintf(message,
                 sizeof(message),
                 "Mirror Nametables H (0x%04x)",
                 addr);
        mu_assert(message, received == expected);
    }
    return 0;
}

static char *test_mirror_nametables_ver() {
    for (unsigned addr = 0x2000; addr < 0x4000; addr++) {
        address_t received = mirror_address_ppu_bus(addr, MIRROR_VERTICAL);
        address_t expected = addr;
        if (expected >= 0x3000) {
            expected -= 0x1000;
        }
        if (expected < PPU_MAP_NAMETABLE_3 && expected >= PPU_MAP_NAMETABLE_2) {
            expected -= 0x800;
        }
        if (expected >= PPU_MAP_NAMETABLE_3) {
            expected -= 0x800;
        }
        snprintf(message,
                 sizeof(message),
                 "Mirror Nametables V (0x%04x)",
                 addr);
        mu_assert(message, received == expected);
    }
    return 0;
}

static char *all_tests() {
    mu_run_test(test_mirror_nametables_hor);
    mu_run_test(test_mirror_nametables_ver);
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