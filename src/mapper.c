#include "./mapper.h"

void create_mapper(mapper_t *mapper, rom_t *rom) {
    mapper->type = rom->header.mapper;
    mapper->rom = rom;

    switch (mapper->type) {
    case MAPPER_NROM:
        break;
    case MAPPER_MMC1:
        create_mmc1(&mapper->state.mmc1);
        break;
    default:
        fprintf(stderr, "Error: Mapper %d not supported\n", mapper->type);
        exit(1);
    }
}

void destroy_mapper(mapper_t *mapper) {
    switch (mapper->type) {
    case MAPPER_NROM:
        break;
    case MAPPER_MMC1:
        destroy_mmc1(&mapper->state.mmc1);
        break;
    default:
        fprintf(stderr, "Error: Mapper %d not supported\n", mapper->type);
        exit(1);
    }
}

unsigned char read_cpu_mapper(mapper_t *mapper, address_t address) {
    switch (mapper->type) {
    case MAPPER_NROM:
        return read_cpu_nrom(mapper->rom, address);
    default:
        fprintf(stderr, "Error: Mapper %d not supported\n", mapper->type);
        exit(1);
    }
}

void write_cpu_mapper(mapper_t *mapper,
                      address_t address,
                      unsigned char value) {
    switch (mapper->type) {
    case MAPPER_NROM:
        write_cpu_nrom(mapper->rom, address, value);
        break;
    default:
        fprintf(stderr, "Error: Mapper %d not supported\n", mapper->type);
        exit(1);
    }
}

unsigned char read_ppu_mapper(mapper_t *mapper, address_t address) {
    switch (mapper->type) {
    case MAPPER_NROM:
        return read_ppu_nrom(mapper->rom, address);
    default:
        fprintf(stderr, "Error: Mapper %d not supported\n", mapper->type);
        exit(1);
    }
}

void write_ppu_mapper(mapper_t *mapper,
                      address_t address,
                      unsigned char value) {
    switch (mapper->type) {
    case MAPPER_NROM:
        write_ppu_nrom(mapper->rom, address, value);
        break;
    default:
        fprintf(stderr, "Error: Mapper %d not supported\n", mapper->type);
        exit(1);
    }
}