#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>

#include "memory.h"

/**
 * @brief Interrupt controller.
 *
 */
typedef struct {
    /**
     * @brief IRQ flag.
     *
     */
    bool irq;

    /**
     * @brief NMI flag, not affected by the CPU I-status.
     *
     */
    bool nmi;

    /**
     * @brief Reset flag.
     *
     */
    bool reset;
} interrupt_t;

#endif