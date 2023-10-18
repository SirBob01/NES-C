#include "./mmc1.h"

void create_mmc1(mmc1_t *mapper) {
    mapper->shift_register = 0;
    mapper->registers[0] = 0;
    mapper->registers[1] = 0;
    mapper->registers[2] = 0;
    mapper->registers[3] = 0;
}

void destroy_mmc1(mmc1_t *mapper) {}

unsigned char read_cpu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address) {
    if (address < 0x8000) {
        return get_prg_ram(rom)[address];
    } else {
        unsigned char mode = (mapper->registers[0] >> 2) & 0x3;
        switch (mode) {
        case 0:
        case 1:
            return get_prg_rom(rom)[address - 0x8000];
        case 2:
        case 3:
            break;
        }
    }
}

unsigned char read_ppu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address) {}

void write_cpu_mmc1(mmc1_t *mapper,
                    rom_t *rom,
                    address_t address,
                    unsigned char value) {
    if (address >= 0x8000) {
        if (value & 0x80) {
            mapper->shift_register = 1 << 4;
        } else {
            if (mapper->shift_register & 1) {
                unsigned char bit = (value & 1) << 4;
                mapper->shift_register >>= 1;
                mapper->shift_register |= bit;

                unsigned char register_index = (address >> 13) & 0x3;
                mapper->registers[register_index] = mapper->shift_register;
                mapper->shift_register = 1 << 4;
            } else {
                unsigned char bit = (value & 1) << 4;
                mapper->shift_register >>= 1;
                mapper->shift_register |= bit;
            }
        }
    } else {
        get_prg_ram(rom)[address] = value;
    }
}

void write_ppu_mmc1(mmc1_t *mapper,
                    rom_t *rom,
                    address_t address,
                    unsigned char value) {
    // NOP
}
