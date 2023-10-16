#include "./controller.h"

void create_controller(controller_t *controller) {
    controller->joypad[0] = 0;
    controller->joypad[1] = 0;

    controller->shift[0] = 0;
    controller->shift[1] = 0;

    controller->joypad_strobe = false;
}

void destroy_controller(controller_t *controller) {}

void write_strobe_controller(controller_t *controller, unsigned char value) {
    controller->joypad_strobe = value & 1;
    if (controller->joypad_strobe) {
        controller->shift[0] = controller->joypad[0];
        controller->shift[1] = controller->joypad[1];
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

void set_joy1_controller(controller_t *controller, joypad_t buttons) {
    controller->joypad[0] = 0;
    controller->joypad[0] |= buttons.a;
    controller->joypad[0] |= buttons.b << 1;
    controller->joypad[0] |= buttons.select << 2;
    controller->joypad[0] |= buttons.start << 3;
    controller->joypad[0] |= buttons.up << 4;
    controller->joypad[0] |= buttons.down << 5;
    controller->joypad[0] |= buttons.left << 6;
    controller->joypad[0] |= buttons.right << 7;
}

void set_joy2_controller(controller_t *controller, joypad_t buttons) {
    controller->joypad[0] = 0;
    controller->joypad[1] |= buttons.a;
    controller->joypad[1] |= buttons.b << 1;
    controller->joypad[1] |= buttons.select << 2;
    controller->joypad[1] |= buttons.start << 3;
    controller->joypad[1] |= buttons.up << 4;
    controller->joypad[1] |= buttons.down << 5;
    controller->joypad[1] |= buttons.left << 6;
    controller->joypad[1] |= buttons.right << 7;
}