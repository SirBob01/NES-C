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
} input_t;

/**
 * @brief Create the input state object.
 *
 * @return input_t
 */
input_t create_input();

/**
 * @brief Poll for user input.
 *
 * @param input
 */
void poll_input(input_t *input);

/**
 * @brief Test for a key press.
 *
 * @param input
 * @param code
 * @return true
 * @return false
 */
bool is_keydown(input_t *input, SDL_Keycode code);

/**
 * @brief Test for a key release.
 *
 * @param input
 * @param code
 * @return true
 * @return false
 */
bool is_keyup(input_t *input, SDL_Keycode code);

#endif