#include "./utils.h"

unsigned char reverse_bits(unsigned char bits) {
    bits = (bits & 0xF0) >> 4 | (bits & 0x0F) << 4;
    bits = (bits & 0xCC) >> 2 | (bits & 0x33) << 2;
    bits = (bits & 0xAA) >> 1 | (bits & 0x55) << 1;
    return bits;
}