#ifndef MAPPER_H
#define MAPPER_H

#include "./cpu.h"
#include "./memory.h"
#include "./rom.h"

/**
 * @brief NROM memory mapper implementation.
 *
 * @param cpu
 * @param rom
 * @param address
 * @return unsigned char*
 */
unsigned char *nrom_mapper(cpu_t *cpu, rom_t *rom, address_t address);

#endif