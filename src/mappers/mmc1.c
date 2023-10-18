#include "./mmc1.h"

void create_mmc1(mmc1_t *mapper) {
    mapper->shift_register = 0;
    mapper->ctrl = 0;
    mapper->chr_bank_0 = 0;
    mapper->chr_bank_1 = 0;
    mapper->prg_bank = 0;
}

void destroy_mmc1(mmc1_t *mapper) {}

unsigned char read_cpu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address) {
    if (address < 0x8000) {
        return get_prg_ram(rom)[address];
    } else {
        unsigned char mode = (mapper->ctrl >> 2) & 0x3;
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

unsigned char read_ppu_mmc1(mmc1_t *mapper, rom_t *rom, address_t address) {
    bool mode = mapper->ctrl & 0x10;
    if (mode) {
        address_t mask = rom->header.chr_rom_size - 1;
        return get_chr_ram(rom)[address & mask];
    } else {
        address_t mask = rom->header.chr_rom_size - 1;
        return get_chr_rom(rom)[address & mask];
    }
}

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

                // Write to the appropriate register
                unsigned char register_index = (address >> 13) & 0x3;
                switch (register_index) {
                case 0:
                    mapper->ctrl = mapper->shift_register;
                    break;
                case 1:
                    mapper->chr_bank_0 = mapper->shift_register;
                    break;
                case 2:
                    mapper->chr_bank_1 = mapper->shift_register;
                    break;
                case 3:
                    mapper->prg_bank = mapper->shift_register;
                    break;
                }
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
