#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "./memory.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

/**
 * @brief Circular buffer.
 *
 */
typedef struct {
    /**
     * @brief Buffer memory.
     *
     */
    memory_t memory;

    /**
     * @brief Read pointer.
     *
     */
    unsigned read;

    /**
     * @brief Write pointer.
     *
     */
    unsigned write;

    /**
     * @brief Masking value.
     *
     */
    unsigned mask;
} buffer_t;

/**
 * @brief Create a buffer.
 *
 * @param capacity
 * @return buffer_t
 */
buffer_t create_buffer(unsigned capacity);

/**
 * @brief Destroy a buffer.
 *
 * @param buffer
 */
void destroy_buffer(buffer_t *buffer);

/**
 * @brief Get the current size of a buffer.
 *
 * @param buffer
 * @return unsigned
 */
unsigned get_size_buffer(buffer_t *buffer);

/**
 * @brief Read up to n values from the buffer, returning the number of elements
 * read.
 *
 * @param buffer
 * @param dst
 * @param n
 * @return unsigned
 */
unsigned read_buffer(buffer_t *buffer, unsigned char *dst, unsigned n);

/**
 * @brief Write up to n values to the buffer, returning the number of elements
 * written.
 *
 * @param buffer
 * @param src
 * @param n
 * @return unsigned
 */
unsigned write_buffer(buffer_t *buffer, unsigned char *src, unsigned n);

/**
 * @brief Clear the buffer.
 *
 * @param buffer
 */
void clear_buffer(buffer_t *buffer);

#endif