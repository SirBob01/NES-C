#include "./cpu_bus.h"

void create_cpu_bus(cpu_bus_t *bus,
                    rom_t *rom,
                    mapper_t *mapper,
                    apu_t *apu,
                    ppu_t *ppu) {
    bus->rom = rom;
    bus->mapper = mapper;
    bus->apu = apu;
    bus->ppu = ppu;
    memset(bus->memory, 0, CPU_RAM_SIZE);
}

address_t mirror_address_cpu_bus(address_t address,
                                 unsigned long prg_ram_size) {
    if (address < CPU_MAP_PPU_REG) {
        // Mirrored RAM region
        return CPU_MAP_START + ((address - CPU_MAP_START) % 0x800);
    } else if (address < CPU_MAP_APU_IO) {
        // Mirrored PPU register memory
        return CPU_MAP_PPU_REG + ((address - CPU_MAP_PPU_REG) % 0x8);
    } else if (address >= CPU_MAP_RAM) {
        // Mirrored PRG RAM memory
        return CPU_MAP_RAM + ((address - CPU_MAP_RAM) % prg_ram_size);
    }
    return address;
}

unsigned char *get_memory_cpu_bus(cpu_bus_t *bus, address_t address) {
    address_t norm_address =
        mirror_address_cpu_bus(address, bus->rom->header.prg_ram_size);
    switch (norm_address) {
    case PPU_REG_CTRL:
        return &bus->ppu->ctrl;
    case PPU_REG_MASK:
        return &bus->ppu->mask;
    case PPU_REG_STATUS:
        return &bus->ppu->status;
    case PPU_REG_OAMADDR:
        return &bus->ppu->oam_addr;
    case PPU_REG_OAMDATA:
        return &bus->ppu->oam_data;
    case PPU_REG_SCROLL:
        return &bus->ppu->scroll;
    case PPU_REG_ADDR:
        return &bus->ppu->addr;
    case PPU_REG_DATA:
        return &bus->ppu->data;
    case PPU_REG_OAMDMA:
        return &bus->ppu->oam_dma;
    case APU_REG_PULSE1_0:
        return &bus->apu->channel_registers.pulse1[0];
    case APU_REG_PULSE1_1:
        return &bus->apu->channel_registers.pulse1[1];
    case APU_REG_PULSE1_2:
        return &bus->apu->channel_registers.pulse1[2];
    case APU_REG_PULSE1_3:
        return &bus->apu->channel_registers.pulse1[3];
    case APU_REG_PULSE2_0:
        return &bus->apu->channel_registers.pulse2[0];
    case APU_REG_PULSE2_1:
        return &bus->apu->channel_registers.pulse2[1];
    case APU_REG_PULSE2_2:
        return &bus->apu->channel_registers.pulse2[2];
    case APU_REG_PULSE2_3:
        return &bus->apu->channel_registers.pulse2[3];
    case APU_REG_TRIANGLE_0:
        return &bus->apu->channel_registers.triangle[0];
    case APU_REG_TRIANGLE_1:
        return &bus->apu->channel_registers.triangle[1];
    case APU_REG_TRIANGLE_2:
        return &bus->apu->channel_registers.triangle[2];
    case APU_REG_TRIANGLE_3:
        return &bus->apu->channel_registers.triangle[3];
    case APU_REG_NOISE_0:
        return &bus->apu->channel_registers.noise[0];
    case APU_REG_NOISE_1:
        return &bus->apu->channel_registers.noise[1];
    case APU_REG_NOISE_2:
        return &bus->apu->channel_registers.noise[2];
    case APU_REG_NOISE_3:
        return &bus->apu->channel_registers.noise[3];
    case APU_REG_DMC_0:
        return &bus->apu->channel_registers.dmc[0];
    case APU_REG_DMC_1:
        return &bus->apu->channel_registers.dmc[1];
    case APU_REG_DMC_2:
        return &bus->apu->channel_registers.dmc[2];
    case APU_REG_DMC_3:
        return &bus->apu->channel_registers.dmc[3];
    case APU_REG_STATUS:
        return &bus->apu->status;
    case APU_REG_FRAME_COUNTER:
        return &bus->apu->frame_counter;
    default:
        return bus->memory + norm_address;
    }
}

unsigned char read_cpu_bus(cpu_bus_t *bus, address_t address) {
    if (address >= CPU_MAP_ROM) {
        return read_cpu_mapper(bus->mapper, address);
    } else {
        unsigned char *ptr = get_memory_cpu_bus(bus, address);
        unsigned char result = *ptr;

        // Read from write only registers (return the latch)
        if (ptr == &bus->ppu->ctrl || ptr == &bus->ppu->mask ||
            ptr == &bus->ppu->oam_addr || ptr == &bus->ppu->scroll ||
            ptr == &bus->ppu->addr || ptr == &bus->ppu->oam_dma) {
            result = bus->ppu->io_databus;
        }

        // Read from PPUSTATUS
        if (ptr == &bus->ppu->status) {
            bus->ppu->status &= ~PPU_STATUS_VBLANK;
            bus->ppu->internal.w = false;

            // Suppress setting VBlank if read too close
            if (bus->ppu->scanline == PPU_SCANLINE_VBLANK &&
                bus->ppu->dot <= 1) {
                bus->ppu->suppress_vbl = true;
            }

            // Set low 5 bits of status register to io latch
            result = (result & 0xE0) | (bus->ppu->io_databus & 0x1F);
            bus->ppu->io_databus = result;
        }

        // Read from OAMDATA (overwrite result with value from primary OAM)
        if (ptr == &bus->ppu->oam_data) {
            result = bus->ppu->sprites.primary_oam[bus->ppu->oam_addr];
            if ((bus->ppu->oam_addr % 4) == 2) {
                result &= 0xE3;
            }
            bus->ppu->io_databus = result;
        }

        // Read from PPUDATA (overwrite result with value from VRAM)
        if (ptr == &bus->ppu->data) {
            result = read_ppu_bus(bus->ppu->bus, bus->ppu->internal.v & 0x3FFF);
            bool inc = bus->ppu->ctrl & PPU_CTRL_VRAM_INC;
            bus->ppu->internal.v += inc ? 32 : 1;

            // Set the high 2 bits from palette to io latch
            if (bus->ppu->internal.v >= 0x3F00) {
                result = (result & 0x3F) | (bus->ppu->io_databus & 0xC0);
            }
            bus->ppu->io_databus = result;
        }

        return result;
    }
}

void write_cpu_bus(cpu_bus_t *bus, address_t address, unsigned char value) {
    if (address >= CPU_MAP_ROM) {
        write_cpu_mapper(bus->mapper, address, value);
    } else {
        unsigned char *ptr = get_memory_cpu_bus(bus, address);
        bool readonly = ptr == &bus->ppu->status;
        if (!readonly) {
            *get_memory_cpu_bus(bus, address) = value;
        }

        // Write to PPUMASK, PPUSTATUS, OAMADDR (set the latch)
        if (ptr == &bus->ppu->mask || ptr == &bus->ppu->status ||
            ptr == &bus->ppu->oam_addr) {
            bus->ppu->io_databus = value;
        }

        // Write to PPUCTRL
        if (ptr == &bus->ppu->ctrl) {
            address_t gh = (value & 0x03) << 10;
            bus->ppu->internal.t = (bus->ppu->internal.t & 0x73FF) | gh;
            bus->ppu->io_databus = value;
        }

        // Write to OAMDATA
        if (ptr == &bus->ppu->oam_data) {
            if (is_rendering_ppu(bus->ppu)) {
                bus->ppu->oam_addr += 0x04;
            } else {
                bus->ppu->sprites.primary_oam[bus->ppu->oam_addr++] = value;
            }
            bus->ppu->io_databus = value;
        }

        // Write to PPUSCROLL
        if (ptr == &bus->ppu->scroll) {
            if (!bus->ppu->internal.w) {
                address_t abcde = (value & 0xF8) >> 3;
                address_t fgh = value & 0x07;

                bus->ppu->internal.t = (bus->ppu->internal.t & 0xFFE0) | abcde;
                bus->ppu->internal.x = fgh;
            } else {
                address_t ab = (value & 0xC0) << 2;
                address_t cde = (value & 0x38) << 2;
                address_t fgh = (value & 0x07) << 12;
                address_t tmask = ab | cde | fgh;

                bus->ppu->internal.t = (bus->ppu->internal.t & 0xC1F) | tmask;
            }
            bus->ppu->internal.w = !bus->ppu->internal.w;
            bus->ppu->io_databus = value;
        }

        // Write to PPUADDR
        if (ptr == &bus->ppu->addr) {
            if (!bus->ppu->internal.w) {
                address_t hi = value & 0x3F;
                address_t lo = bus->ppu->internal.t & 0x00FF;
                bus->ppu->internal.t = (hi << 8) | lo;
            } else {
                bus->ppu->internal.t = (bus->ppu->internal.t & 0x7F00) | value;
                bus->ppu->internal.v = bus->ppu->internal.t;
            }
            bus->ppu->internal.w = !bus->ppu->internal.w;
            bus->ppu->io_databus = value;
        }

        // Write to PPUDATA
        if (ptr == &bus->ppu->data) {
            write_ppu_bus(bus->ppu->bus, bus->ppu->internal.v & 0x3FFF, value);
            bool inc = bus->ppu->ctrl & PPU_CTRL_VRAM_INC;
            bus->ppu->internal.v += inc ? 32 : 1;
            bus->ppu->io_databus = value;
        }

        // Write to OAMDMA (TODO: fix cycle timing, make this a CPU routine?)
        if (ptr == &bus->ppu->oam_dma) {
            address_t dma_addr = value << 8;
            for (unsigned i = 0; i < 256; i++) {
                // Get
                unsigned char data = read_cpu_bus(bus, dma_addr + i);

                // Put
                write_cpu_bus(bus, PPU_REG_OAMDATA, data);
            }
            bus->ppu->io_databus = value;
        }
    }
}

unsigned
read_string_cpu_bus(cpu_bus_t *bus, address_t address, char *dst, unsigned n) {
    char c = read_cpu_bus(bus, address);
    unsigned length = 0;
    while (c && length < n - 1) {
        dst[length++] = c;
        c = read_cpu_bus(bus, ++address);
    }
    dst[length] = 0;
    return length;
}
