#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>

/**
 * @brief Buttons of the standard NES joypad.
 *
 */
typedef struct {
    bool a;
    bool b;
    bool select;
    bool start;
    bool up;
    bool down;
    bool left;
    bool right;
} joypad_t;

/**
 * @brief NES input controller state.
 *
 */
typedef struct {
    /**
     * @brief Button states of the joypad controllers.
     *
     */
    unsigned char joypad[2];

    /**
     * @brief Shift registers for each controller.
     *
     */
    unsigned char shift[2];

    /**
     * @brief Flag that determines if the controller input should be polled.
     *
     */
    bool joypad_strobe;
} controller_t;

/**
 * @brief Create a controller object.
 *
 * @param controller
 */
void create_controller(controller_t *controller);

/**
 * @brief Destroy a controller object.
 *
 * @param controller
 */
void destroy_controller(controller_t *controller);

/**
 * @brief Write to $4016.
 *
 * @param controller
 * @param strobe
 */
void write_strobe_controller(controller_t *controller, unsigned char value);

/**
 * @brief Read the button states of the first controller.
 *
 * @param controller
 * @return unsigned char
 */
unsigned char read_joy1_controller(controller_t *controller);

/**
 * @brief Read the button states of the second controller.
 *
 * @param controller
 * @return unsigned char
 */
unsigned char read_joy2_controller(controller_t *controller);

/**
 * @brief Set the button states of the first controller.
 *
 * @param controller
 * @param buttons
 */
void set_joy1_controller(controller_t *controller, joypad_t buttons);

/**
 * @brief Set the button states of the second controller.
 *
 * @param controller
 * @param buttons
 */
void set_joy2_controller(controller_t *controller, joypad_t buttons);

#endif