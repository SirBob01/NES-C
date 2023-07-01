#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>

#include "memory.h"

/**
 * @brief Interrupt state.
 *
 */
typedef struct {
    /**
     * @brief IRQ line.
     *
     */
    bool irq;

    /**
     * @brief NMI line, not affected by the CPU I-flag.
     *
     */
    bool nmi;

    /**
     * @brief Reset line.
     *
     */
    bool reset;
} interrupt_t;

#endif