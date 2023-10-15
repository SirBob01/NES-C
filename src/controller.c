#include "./controller.h"

void create_controller(controller_t *controller) {
    controller->joy[0] = 0;
    controller->joy[1] = 0;

    controller->shift[0] = 0;
    controller->shift[1] = 0;

    controller->strobe = false;
}

void destroy_controller(controller_t *controller) {}

void write_strobe_controller(controller_t *controller, unsigned char value) {
    controller->strobe = value & 1;
    if (controller->strobe) {
        controller->shift[0] = controller->joy[0];
        controller->shift[1] = controller->joy[1];
    }
}

unsigned char read_joy1_controller(controller_t *controller) {
    unsigned char result = controller->shift[0] & 1;
    controller->shift[0] >>= 1;
    controller->shift[0] |= 0x80;
    return result;
}

unsigned char read_joy2_controller(controller_t *controller) {
    unsigned char result = controller->shift[1] & 1;
    controller->shift[1] >>= 1;
    controller->shift[1] |= 0x80;
    return result;
}

void set_joy1_controller(controller_t *controller,
                         bool a,
                         bool b,
                         bool select,
                         bool start,
                         bool up,
                         bool down,
                         bool left,
                         bool right) {
    controller->joy[0] = 0;
    controller->joy[0] |= a;
    controller->joy[0] |= b << 1;
    controller->joy[0] |= select << 2;
    controller->joy[0] |= start << 3;
    controller->joy[0] |= up << 4;
    controller->joy[0] |= down << 5;
    controller->joy[0] |= left << 6;
    controller->joy[0] |= right << 7;
}

void set_joy2_controller(controller_t *controller,
                         bool a,
                         bool b,
                         bool select,
                         bool start,
                         bool up,
                         bool down,
                         bool left,
                         bool right) {
    controller->joy[0] = 0;
    controller->joy[1] |= a;
    controller->joy[1] |= b << 1;
    controller->joy[1] |= select << 2;
    controller->joy[1] |= start << 3;
    controller->joy[1] |= up << 4;
    controller->joy[1] |= down << 5;
    controller->joy[1] |= left << 6;
    controller->joy[1] |= right << 7;
}