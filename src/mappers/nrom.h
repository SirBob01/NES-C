#ifndef MAPPER_NROM_H
#define MAPPER_NROM_H

#include "../cpu_bus.h"
#include "../memory.h"
#include "../ppu_bus.h"
#include "../rom.h"

/**
 * @brief NROM CPU memory mapper.
 *
 * @param bus
 * @param address
 * @return unsigned char*
 */
unsigned char *nrom_cpu(cpu_bus_t *bus, address_t address);

/**
 * @brief NROM PPU memory mapper.
 *
 * @param bus
 * @param address
 * @return unsigned char*
 */
unsigned char *nrom_ppu(ppu_bus_t *bus, address_t address);

#endif