#include "./color.h"

color_t create_color(unsigned char palette_byte,
                     bool grayscale,
                     bool em_r,
                     bool em_g,
                     bool em_b) {
    if (grayscale) {
        palette_byte &= 0x30;
    }

    // Apply emphasis
    // TODO: Implement accurate NTSC signal emulation
    color_t color = COLOR_PALETTE[palette_byte];
    if (em_r) {
        color.r = min(color.r * 1.5, 0xff);
        color.g = color.g * 0.5;
        color.b = color.b * 0.5;
    }
    if (em_g) {
        color.r = color.r * 0.5;
        color.g = min(color.g * 1.5, 0xff);
        color.b = color.b * 0.5;
    }
    if (em_b) {
        color.r = color.r * 0.5;
        color.g = color.g * 0.5;
        color.b = min(color.b * 1.5, 0xff);
    }
    return color;
}
