#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>

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

/**
 * @brief Reset the interrupt state.
 *
 * @param interrupt
 */
void reset_interrupt(interrupt_t *interrupt);

#endif