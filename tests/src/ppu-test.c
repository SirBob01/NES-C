#include <stdio.h>
#include <string.h>

#include "./ctest.h"

#include "../../src/emulator.h"

int tests_run = 0;

static char *test_blargg_ppu_vbl_nmi() {
    const char *test_roms[13] = {
        "../roms/ppu_open_bus/ppu_open_bus.nes",
        "../roms/oam_read/oam_read.nes",
        "../roms/oam_stress/oam_stress.nes",
        "../roms/ppu_vbl_nmi/01-vbl_basics.nes",
        "../roms/ppu_vbl_nmi/02-vbl_set_time.nes",
        "../roms/ppu_vbl_nmi/03-vbl_clear_time.nes",
        "../roms/ppu_vbl_nmi/04-nmi_control.nes",
        "../roms/ppu_vbl_nmi/05-nmi_timing.nes",
        "../roms/ppu_vbl_nmi/06-suppression.nes",
        "../roms/ppu_vbl_nmi/07-nmi_on_timing.nes",
        "../roms/ppu_vbl_nmi/08-nmi_off_timing.nes",
        "../roms/ppu_vbl_nmi/09-even_odd_frames.nes",
        "../roms/ppu_vbl_nmi/10-even_odd_timing.nes",
    };

    // Number of cycles before timing out
    const unsigned long CYCLE_TIMEOUT = 100000000;

    // Test results
    unsigned char status = 0;
    char result[2048] = {0};

    for (unsigned i = 0; i < 13; i++) {
        emulator_t emu;
        create_emulator(&emu, test_roms[i]);

        bool verified = false;
        for (unsigned cycles = 0; cycles < CYCLE_TIMEOUT; cycles++) {
            bool running = update_emulator(&emu);
            mu_assert("PPU_VBL_NMI emulator fatally crashed.", running);

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

        printf("PPU_VBL_NMI %s\n", test_roms[i]);
        printf("Result: %02X\n", status);
        printf("%s\n", result);

        mu_assert("PPU_VBL_NMI result not successful",
                  strstr(result, "Passed"));

        destroy_emulator(&emu);
    }
    return 0;
}

static char *all_tests() {
    mu_run_test(test_blargg_ppu_vbl_nmi);
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