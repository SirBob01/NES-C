#include <stdio.h>
#include <string.h>

#include "./ctest.h"

#include "../../src/emulator.h"

int tests_run = 0;

static char *test_nestest() {
    emulator_t emu;
    create_emulator(&emu, "../roms/nestest/nestest.nes");
    emu.cpu.pc = 0xC000;

    FILE *file = fopen("../roms/nestest/nestest.log", "r");
    mu_assert("NESTEST Could not open nestest.log", file != NULL);

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
    unsigned line_num = 1;
    while (running && lines) {
        fgets(src, sizeof(src), file);
        src[strlen(src) - 2] = 0;
        read_state_cpu(&emu.cpu, dst, sizeof(dst));
        printf("L%d %s %lu\n", line_num, src, strlen(src));
        printf("L%d %s %lu\n\n", line_num, dst, strlen(dst));
        mu_assert("NESTEST CPU state does not match nestest.log",
                  strcmp(src, dst) == 0);
        running = update_emulator(&emu);
        lines--;
        line_num++;
    }

    // Read the final test result in 0x2
    mu_assert("NESTEST result is not succesful",
              read_cpu_bus(&emu.cpu_bus, 0x2) == 0x00);

    destroy_emulator(&emu);
    fclose(file);
    return 0;
}

static char *test_blargg_instr_test_v5() {
    const char *test_roms[20] = {
        "../roms/instr_test_v5/01-basics.nes",
        "../roms/instr_test_v5/02-implied.nes",
        "../roms/instr_test_v5/03-immediate.nes",
        "../roms/instr_test_v5/04-zero_page.nes",
        "../roms/instr_test_v5/05-zp_xy.nes",
        "../roms/instr_test_v5/06-absolute.nes",
        "../roms/instr_test_v5/07-abs_xy.nes",
        "../roms/instr_test_v5/08-ind_x.nes",
        "../roms/instr_test_v5/09-ind_y.nes",
        "../roms/instr_test_v5/10-branches.nes",
        "../roms/instr_test_v5/11-stack.nes",
        "../roms/instr_test_v5/12-jmp_jsr.nes",
        "../roms/instr_test_v5/13-rts.nes",
        "../roms/instr_test_v5/14-rti.nes",
        "../roms/instr_test_v5/15-brk.nes",
        "../roms/instr_test_v5/16-special.nes",
        "../roms/instr_misc/01-abs_x_wrap.nes",
        "../roms/instr_misc/02-branch_wrap.nes",

        // TODO: Run these as soon as PPU is setup
        "../roms/instr_misc/03-dummy_reads.nes",
        "../roms/instr_misc/04-dummy_reads_apu.nes",
    };

    // Number of cycles before timing out
    const unsigned long CYCLE_TIMEOUT = 100000000;

    // Test results
    unsigned char status = 0;
    char result[64] = {0};

    for (unsigned i = 0; i < 18; i++) {
        emulator_t emu;
        create_emulator(&emu, test_roms[i]);

        bool verified = false;
        for (unsigned cycles = 0; cycles < CYCLE_TIMEOUT; cycles++) {
            bool running = update_emulator(&emu);
            mu_assert("INSTR_TEST_V5 emulator fatally crashed.", running);

            if (!verified) {
                unsigned char m0 = read_cpu_bus(&emu.cpu_bus, 0x6001);
                unsigned char m1 = read_cpu_bus(&emu.cpu_bus, 0x6002);
                unsigned char m2 = read_cpu_bus(&emu.cpu_bus, 0x6003);

                // Verify the magic numbers and restart the loop
                if (m0 == 0xDE && m1 == 0xB0 && m2 == 0x61) {
                    verified = true;
                    cycles = 0;
                }
            } else {
                status = read_cpu_bus(&emu.cpu_bus, 0x6000);
                read_string_cpu_bus(&emu.cpu_bus,
                                    0x6004,
                                    result,
                                    sizeof(result));
                if ((strstr(result, "Passed") || strstr(result, "Failed")) &&
                    status <= 0x7F) {
                    break;
                }
            }
        }

        printf("INSTR_TEST_V5 %s\n", test_roms[i]);
        printf("Result: %02X\n", status);
        printf("%s\n", result);

        mu_assert("INSTR_TEST_V5 result not successful",
                  strstr(result, "Passed"));

        destroy_emulator(&emu);
    }
    return 0;
}

static char *all_tests() {
    mu_run_test(test_nestest);
    mu_run_test(test_blargg_instr_test_v5);
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