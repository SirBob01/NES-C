#include "./ppu.h"

ppu_t *create_ppu() {
    ppu_t *ppu = (ppu_t *)malloc(sizeof(ppu_t));
    ppu->ctrl = 0;
    ppu->mask = 0;
    ppu->status = 0;
    ppu->oam_addr = 0;
    ppu->oam_data = 0;
    ppu->scroll = 0;
    ppu->addr = 0;
    ppu->data = 0;
    ppu->oam_dma = 0;
    return ppu;
}

void destroy_ppu(ppu_t *ppu) { free(ppu); }

void update_ppu(ppu_t *ppu) {
    // TODO: Implement this.
}