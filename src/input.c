#include "./input.h"

input_t create_input() {
    input_t input;
    input.quit = false;
    return input;
}

void poll_input(input_t *input) {
    SDL_PollEvent(&input->event);
    if (input->event.type == SDL_EVENT_QUIT) {
        input->quit = true;
    }
}

bool is_keydown(input_t *input, SDL_Keycode code) {
    if (input->event.type == SDL_EVENT_KEY_DOWN) {
        return input->event.key.keysym.sym == code;
    } else {
        return false;
    }
}

bool is_keyup(input_t *input, SDL_Keycode code) {
    if (input->event.type == SDL_EVENT_KEY_UP) {
        return input->event.key.keysym.sym == code;
    } else {
        return false;
    }
}
