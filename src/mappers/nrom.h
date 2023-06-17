#ifndef MAPPER_NROM_H
#define MAPPER_NROM_H

#include "../cpu.h"
#include "../memory.h"
#include "../rom.h"

/**
 * @brief NROM CPU memory mapper.
 *
 * @param cpu
 * @param address
 * @return unsigned char*
 */
unsigned char *nrom_cpu(cpu_t *cpu, address_t address);

/**
 * @brief NROM PPU memory mapper.
 *
 * @param ppu
 * @param address
 * @return unsigned char*
 */
unsigned char *nrom_ppu(ppu_t *ppu, address_t address);

#endif