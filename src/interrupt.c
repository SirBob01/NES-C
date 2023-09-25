#include "./interrupt.h"

void reset_interrupt(interrupt_t *interrupt) {
    set_irq_interrupt(interrupt, false);
    set_nmi_interrupt(interrupt, false);
    set_reset_interrupt(interrupt, false);
}

bool get_irq_interrupt(interrupt_t *interrupt) { return interrupt->irq; }

bool get_nmi_interrupt(interrupt_t *interrupt) { return interrupt->nmi; }

bool get_reset_interrupt(interrupt_t *interrupt) { return interrupt->reset; }

void set_irq_interrupt(interrupt_t *interrupt, bool state) {
    interrupt->irq = state;
}

void set_nmi_interrupt(interrupt_t *interrupt, bool state) {
    interrupt->nmi = state;
}

void set_reset_interrupt(interrupt_t *interrupt, bool state) {
    interrupt->reset = state;
}