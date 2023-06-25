#include "./ppu.h"
#include "./mappers/nrom.h"

ppu_t *create_ppu(rom_t *rom) {
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

    ppu->scanline = 0;
    ppu->dot = 21; // Initial cycle count
    ppu->odd_frame = false;

    ppu->memory = allocate_memory(PPU_RAM_SIZE);
    ppu->rom = rom;
    return ppu;
}

void destroy_ppu(ppu_t *ppu) {
    free_memory(&ppu->memory);
    free(ppu);
}

address_t mirror_address_ppu(address_t address, rom_mirroring_t mirroring) {
    if (address < PPU_MAP_NAMETABLE_MIRROR && address >= PPU_MAP_NAMETABLE_0) {
        // Mirrored nametable regions
        switch (mirroring) {
        case MIRROR_VERTICAL:
            return PPU_MAP_NAMETABLE_0 +
                   ((address - PPU_MAP_NAMETABLE_0) % 0x800);
        case MIRROR_HORIZONTAL: {
            if (address < PPU_MAP_NAMETABLE_2) {
                return PPU_MAP_NAMETABLE_0 +
                       ((address - PPU_MAP_NAMETABLE_0) % 0x400);
            } else {
                return PPU_MAP_NAMETABLE_2 +
                       ((address - PPU_MAP_NAMETABLE_2) % 0x400);
            }
        }
        }
    } else if (address >= PPU_MAP_PALETTE && address < PPU_MAP_PALETTE_MIRROR) {
        // Mirrored palette table regions
        switch (address) {
        case 0x3f10:
            return 0x3f00;
        case 0x3f14:
            return 0x3f04;
        case 0x3f18:
            return 0x3f08;
        case 0x3f1c:
            return 0x3f0c;
        default:
            return address;
        }
    } else if (address >= PPU_MAP_PALETTE_MIRROR) {
        // Mirrored palette table regions
        address_t norm = PPU_MAP_PALETTE + ((address - PPU_MAP_PALETTE) % 0x20);
        switch (norm) {
        case 0x3f10:
            return 0x3f00;
        case 0x3f14:
            return 0x3f04;
        case 0x3f18:
            return 0x3f08;
        case 0x3f1c:
            return 0x3f0c;
        default:
            return norm;
        }
    }
    return address;
}

unsigned char *apply_memory_mapper_ppu(ppu_t *ppu, address_t address) {
    switch (ppu->rom->header.mapper) {
    case 0:
        return nrom_ppu(ppu, address);
    default:
        fprintf(stderr,
                "Error: Unsupported PPU mapper %d\n",
                ppu->rom->header.mapper);
        exit(1);
    }
}

unsigned char *get_memory_ppu(ppu_t *ppu, address_t address) {
    if (address >= PPU_MAP_NAMETABLE_0) {
        rom_mirroring_t mirroring = ppu->rom->header.mirroring;
        return ppu->memory.buffer + mirror_address_ppu(address, mirroring);
    }
    return apply_memory_mapper_ppu(ppu, address);
}

void read_state_ppu(ppu_t *ppu, char *buffer, unsigned buffer_size) {
    snprintf(buffer,
             buffer_size,
             "CTRL:%02X STATUS:%02X MASK:%02X SCROLL:%02X ADDR:%02X DATA:%02X "
             "OAMADDR:%02X OAMDATA:%02X CYC:%3d,%3d",
             ppu->ctrl,
             ppu->status,
             ppu->mask,
             ppu->scroll,
             ppu->addr,
             ppu->data,
             ppu->oam_addr,
             ppu->oam_data,
             ppu->scanline,
             ppu->dot);
}

void update_ppu(ppu_t *ppu) {
    ppu->dot++;
    if (ppu->dot >= PPU_LINEDOTS) {
        ppu->dot = 0;
        ppu->scanline++;
    }
    if (ppu->scanline >= PPU_SCANLINES) {
        ppu->scanline = 0;
        ppu->odd_frame = !ppu->odd_frame;
    }
}