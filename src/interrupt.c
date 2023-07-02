#include "./interrupt.h"

void reset_interrupt(interrupt_t *interrupt) {
    interrupt->irq = false;
    interrupt->nmi = false;
    interrupt->reset = false;
}
