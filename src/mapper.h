#ifndef MAPPER_H
#define MAPPER_H

#include "./mappers/mmc1.h"
#include "./mappers/nrom.h"
#include "./memory.h"
#include "./rom.h"

/**
 * @brief Enumeration of all supported mapper types.
 *
 */
typedef enum {
    MAPPER_NROM = 0,
    MAPPER_MMC1 = 1,
} mapper_type_t;

/**
 * @brief Mapper states.
 *
 */
typedef struct {
    /**
     * @brief Mapper type ID.
     *
     */
    mapper_type_t type;

    /**
     * @brief Pointer to the ROM.
     *
     */
    rom_t *rom;

    /**
     * @brief Mapper state.
     *
     */
    union {
        mmc1_t mmc1;
    } state;
} mapper_t;

/**
 * @brief Create a mapper object.
 *
 * @param mapper
 * @param rom
 */
void create_mapper(mapper_t *mapper, rom_t *rom);

/**
 * @brief Destroy a mapper object.
 *
 * @param mapper
 * @param mapper_id
 */
void destroy_mapper(mapper_t *mapper);

/**
 * @brief Read from a CPU mapper.
 *
 * @param mapper
 * @param address
 * @return unsigned char
 */
unsigned char read_cpu_mapper(mapper_t *mapper, address_t address);

/**
 * @brief Write to a CPU mapper.
 *
 * @param mapper
 * @param address
 * @param value
 */
void write_cpu_mapper(mapper_t *mapper, address_t address, unsigned char value);

/**
 * @brief Read from a PPU mapper.
 *
 * @param mapper
 * @param mapper_id
 * @param address
 * @return unsigned char
 */
unsigned char read_ppu_mapper(mapper_t *mapper, address_t address);

/**
 * @brief Write to a PPU mapper.
 *
 * @param mapper
 * @param mapper_id
 * @param address
 * @param value
 */
void write_ppu_mapper(mapper_t *mapper, address_t address, unsigned char value);

#endif