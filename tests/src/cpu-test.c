#include <stdio.h>
#include <string.h>

#include "./ctest.h"

#include "../../src/emulator.h"

int tests_run = 0;

static char *test_opcodes() {
    emulator_t *emu = create_emulator("../roms/nestest.nes");
    emu->cpu->pc = 0xC000;

    FILE *file = fopen("../roms/nestest.log", "r");
    mu_assert("Could not open nestest.log", file != NULL);

    // Get line count
    unsigned long lines = 0;
    for (char c = getc(file); c != EOF; c = getc(file)) {
        lines += (c == '\n');
    }
    fseek(file, 0, SEEK_SET);

    // Compare each line with runtime CPU state
    char src[256];
    char dst[256];
    bool running = true;
    while (running && lines) {
        fgets(src, sizeof(src), file);
        src[strlen(src) - 2] = 0;
        read_state_cpu(emu->cpu, dst, sizeof(dst));
        printf("%s %lu\n", src, strlen(src));
        printf("%s %lu\n\n", dst, strlen(dst));
        mu_assert("CPU state does not match nestest.log",
                  strcmp(src, dst) == 0);
        running = update_cpu(emu->cpu);
        lines--;
    }

    destroy_emulator(emu);
    fclose(file);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_opcodes);
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