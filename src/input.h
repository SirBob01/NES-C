#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Input state.
 *
 */
typedef struct {
    SDL_Event event;
    bool quit;
    const unsigned char *keystate;
} input_t;

/**
 * @brief Create a input state object.
 *
 * @return input_t*
 */
input_t *create_input();

/**
 * @brief Destroy a input state object.
 *
 * @param input
 */
void destroy_input(input_t *input);

/**
 * @brief Poll for user input.
 *
 * @param input
 */
void poll_input(input_t *input);

/**
 * @brief Test if a key is pressed.
 *
 * @param input
 * @param code
 * @return true
 * @return false
 */
bool is_keydown_input(input_t *input, SDL_Keycode code);

#endif