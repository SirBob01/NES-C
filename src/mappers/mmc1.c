#include "./mmc1.h"

void create_mmc1(mmc1_t *mapper) {
    mapper->shift_register = 0;
    mapper->write_counter = 0;
}

void destroy_mmc1(mmc1_t *mapper) {}
