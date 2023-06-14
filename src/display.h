#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "./memory.h"

/**
 * @brief 2D Vector.
 *
 */
typedef struct {
    float x;
    float y;
} vec2_t;

/**
 * @brief RGBA Color.
 *
 */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} color_t;

/**
 * @brief 2D Display.
 *
 */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    memory_t bitmap;
    vec2_t size;
} display_t;

/**
 * @brief Create a 2D display.
 *
 * @param width  Logical width of the display.
 * @param height Logical height of the display.
 * @param title  Title of the window.
 * @return display_t
 */
display_t create_display(unsigned width, unsigned height, const char *title);

/**
 * @brief Destroy a 2D display.
 *
 * @param display
 */
void destroy_display(display_t *display);

/**
 * @brief Refresh the display.
 *
 * @param display
 */
void refresh_display(display_t *display);

/**
 * @brief Fill the display with a color.
 *
 * @param display
 * @param color
 */
void render_fill(display_t *display, color_t color);

/**
 * @brief Draw a pixel at a position.
 *
 * @param display
 * @param position
 * @param color
 */
void render_pixel(display_t *display, vec2_t position, color_t color);

#endif
