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
     * @brief RESET line.
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

/**
 * @brief Get the IRQ interrupt status.
 *
 * @param interrupt
 * @return true
 * @return false
 */
bool get_irq_interrupt(interrupt_t *interrupt);

/**
 * @brief Get the NMI interrupt edge state.
 *
 * @param interrupt
 * @return true
 * @return false
 */
bool get_nmi_interrupt(interrupt_t *interrupt);

/**
 * @brief Get the RESET interrupt edge state.
 *
 * @param interrupt
 * @return true
 * @return false
 */
bool get_reset_interrupt(interrupt_t *interrupt);

/**
 * @brief Set the IRQ interrupt state.
 *
 * @param interrupt
 * @param state
 */
void set_irq_interrupt(interrupt_t *interrupt, bool state);

/**
 * @brief Set the NMI interrupt state.
 *
 * @param interrupt
 * @param state
 */
void set_nmi_interrupt(interrupt_t *interrupt, bool state);

/**
 * @brief Set the RESET interrupt state.
 *
 * @param interrupt
 * @param state
 */
void set_reset_interrupt(interrupt_t *interrupt, bool state);

#endif