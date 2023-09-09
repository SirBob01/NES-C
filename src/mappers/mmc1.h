#ifndef MAPPER_MMC1_H
#define MAPPER_MMC1_H

#define MMC1_SERIAL_PORT 0x8000

/**
 * @brief MMC1 mapper state.
 *
 */
typedef struct {
    /**
     * @brief Common shift register connected to the serial port.
     *
     */
    unsigned char shift_register;

    /**
     * @brief Write counter.
     *
     */
    unsigned long write_counter;
} mmc1_t;

/**
 * @brief Create the MMC1 mapper state.
 *
 * @param mapper
 */
void create_mmc1(mmc1_t *mapper);

/**
 * @brief Destroy
 *
 * @param mapper
 */
void destroy_mmc1(mmc1_t *mapper);

#endif