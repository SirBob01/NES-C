#include <stdio.h>

#include "./ctest.h"

#include "../../src/emulator.h"

int tests_run = 0;

static char *test_mirror_ram() {
    // RAM
    mu_assert("RAM (L)", mirror_address_cpu_bus(0x0000) == 0);
    mu_assert("RAM (M)", mirror_address_cpu_bus(0x0050) == 0x050);
    mu_assert("RAM (H)", mirror_address_cpu_bus(0x07ff) == 0x07ff);

    // Mirror 0
    mu_assert("RAM Mirror 0 (L)", mirror_address_cpu_bus(0x0800) == 0);
    mu_assert("RAM Mirror 0 (M)", mirror_address_cpu_bus(0x0900) == 0x0100);
    mu_assert("RAM Mirror 0 (H)", mirror_address_cpu_bus(0x0fff) == 0x07ff);

    // Mirror 1
    mu_assert("RAM Mirror 1 (L)", mirror_address_cpu_bus(0x1000) == 0);
    mu_assert("RAM Mirror 1 (M)", mirror_address_cpu_bus(0x1100) == 0x0100);
    mu_assert("RAM Mirror 1 (H)", mirror_address_cpu_bus(0x17ff) == 0x07ff);

    // Mirror 2
    mu_assert("RAM Mirror 2 (L)", mirror_address_cpu_bus(0x1800) == 0);
    mu_assert("RAM Mirror 2 (M)", mirror_address_cpu_bus(0x1900) == 0x0100);
    mu_assert("RAM Mirror 2 (H)", mirror_address_cpu_bus(0x1fff) == 0x07ff);
    return 0;
}

static char *test_mirror_ppu() {
    // PPU
    mu_assert("PPU (L)", mirror_address_cpu_bus(0x2000) == 0x2000);
    mu_assert("PPU (M)", mirror_address_cpu_bus(0x2004) == 0x2004);
    mu_assert("PPU (H)", mirror_address_cpu_bus(0x2007) == 0x2007);

    // First Mirror
    mu_assert("PPU Mirror 0 (L)", mirror_address_cpu_bus(0x2008) == 0x2000);
    mu_assert("PPU Mirror 0 (M)", mirror_address_cpu_bus(0x200c) == 0x2004);
    mu_assert("PPU Mirror 0 (H)", mirror_address_cpu_bus(0x200f) == 0x2007);

    // Last Mirror
    mu_assert("PPU Mirror last (L)", mirror_address_cpu_bus(0x3ff8) == 0x2000);
    mu_assert("PPU Mirror last (M)", mirror_address_cpu_bus(0x3ffc) == 0x2004);
    mu_assert("PPU Mirror last (H)", mirror_address_cpu_bus(0x3fff) == 0x2007);
    return 0;
}

static char *test_mirror_apu() {
    mu_assert("APU I/O (L)", mirror_address_cpu_bus(0x4000) == 0x4000);
    mu_assert("APU I/O (M)", mirror_address_cpu_bus(0x4017) == 0x4017);
    mu_assert("APU I/O (H)", mirror_address_cpu_bus(0x4019) == 0x4019);
    return 0;
}

static char *test_map_ppu_registers() {
    emulator_t emu;
    create_emulator(&emu, "../roms/nestest/nestest.nes");
    mu_assert("PPU CTRL",
              &emu.ppu.ctrl == get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_CTRL));
    mu_assert("PPU MASK",
              &emu.ppu.mask == get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_MASK));
    mu_assert("PPU STATUS",
              &emu.ppu.status ==
                  get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_STATUS));
    mu_assert("PPU OAM ADDR",
              &emu.ppu.oam_addr ==
                  get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_OAMADDR));
    mu_assert("PPU OAM DATA",
              &emu.ppu.oam_data ==
                  get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_OAMDATA));
    mu_assert("PPU SCROLL",
              &emu.ppu.scroll ==
                  get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_SCROLL));
    mu_assert("PPU ADDR",
              &emu.ppu.addr == get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_ADDR));
    mu_assert("PPU DATA",
              &emu.ppu.data == get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_DATA));
    mu_assert("PPU OAM DMA",
              &emu.ppu.oam_dma ==
                  get_memory_cpu_bus(&emu.cpu_bus, PPU_REG_OAMDMA));
    destroy_emulator(&emu);
    return 0;
}

static char *test_map_apu_registers() {
    emulator_t emu;
    create_emulator(&emu, "../roms/nestest/nestest.nes");
    mu_assert("APU PULSE1 0",
              &emu.apu.channel_registers.pulse1[0] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE1_0));
    mu_assert("APU PULSE1 1",
              &emu.apu.channel_registers.pulse1[1] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE1_1));
    mu_assert("APU PULSE1 2",
              &emu.apu.channel_registers.pulse1[2] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE1_2));
    mu_assert("APU PULSE1 3",
              &emu.apu.channel_registers.pulse1[3] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE1_3));
    mu_assert("APU PULSE2 0",
              &emu.apu.channel_registers.pulse2[0] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE2_0));
    mu_assert("APU PULSE2 1",
              &emu.apu.channel_registers.pulse2[1] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE2_1));
    mu_assert("APU PULSE2 2",
              &emu.apu.channel_registers.pulse2[2] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE2_2));
    mu_assert("APU PULSE2 3",
              &emu.apu.channel_registers.pulse2[3] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_PULSE2_3));
    mu_assert("APU TRIANGLE 0",
              &emu.apu.channel_registers.triangle[0] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_TRIANGLE_0));
    mu_assert("APU TRIANGLE 1",
              &emu.apu.channel_registers.triangle[1] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_TRIANGLE_1));
    mu_assert("APU TRIANGLE 2",
              &emu.apu.channel_registers.triangle[2] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_TRIANGLE_2));
    mu_assert("APU TRIANGLE 3",
              &emu.apu.channel_registers.triangle[3] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_TRIANGLE_3));
    mu_assert("APU NOISE 0",
              &emu.apu.channel_registers.noise[0] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_NOISE_0));
    mu_assert("APU NOISE 1",
              &emu.apu.channel_registers.noise[1] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_NOISE_1));
    mu_assert("APU NOISE 2",
              &emu.apu.channel_registers.noise[2] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_NOISE_2));
    mu_assert("APU NOISE 3",
              &emu.apu.channel_registers.noise[3] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_NOISE_3));
    mu_assert("APU DMC 0",
              &emu.apu.channel_registers.dmc[0] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_DMC_0));
    mu_assert("APU DMC 1",
              &emu.apu.channel_registers.dmc[1] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_DMC_1));
    mu_assert("APU DMC 2",
              &emu.apu.channel_registers.dmc[2] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_DMC_2));
    mu_assert("APU DMC 3",
              &emu.apu.channel_registers.dmc[3] ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_DMC_3));
    mu_assert("APU STATUS",
              &emu.apu.status ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_STATUS));
    mu_assert("APU FRAME COUNTER",
              &emu.apu.frame_counter ==
                  get_memory_cpu_bus(&emu.cpu_bus, APU_REG_FRAME_COUNTER));

    destroy_emulator(&emu);
    return 0;
}

static char *test_mapper0() {
    emulator_t emu;
    create_emulator(&emu, "../roms/nestest/nestest.nes");

    unsigned short pc = read_short_cpu_bus(&emu.cpu_bus, CPU_VEC_RESET);
    mu_assert("MAPPER 0 RESET VECTOR", pc == 0xc004);

    destroy_emulator(&emu);
    return 0;
}

static char *test_stack() {
    emulator_t emu;
    create_emulator(&emu, "../roms/nestest/nestest.nes");
    mu_assert("Stack address start", emu.cpu.registers.s == 0xfd);

    push_byte_cpu(&emu.cpu, 0x3);
    mu_assert("Stack address after push", emu.cpu.registers.s == 0xfc);

    mu_assert("Stack pop top byte", pull_byte_cpu(&emu.cpu) == 0x3);
    mu_assert("Stack address end", emu.cpu.registers.s == 0xfd);

    destroy_emulator(&emu);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_mirror_ram);
    mu_run_test(test_mirror_ppu);
    mu_run_test(test_mirror_apu);
    mu_run_test(test_map_ppu_registers);
    mu_run_test(test_map_apu_registers);
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