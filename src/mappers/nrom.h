#ifndef MAPPER_NROM_H
#define MAPPER_NROM_H

#include "../cpu.h"
#include "../memory.h"
#include "../rom.h"

/**
 * @brief NROM CPU memory mapper implementation.
 *
 * @param cpu
 * @param address
 * @return unsigned char*
 */
unsigned char *nrom_cpu(cpu_t *cpu, address_t address);

#endif