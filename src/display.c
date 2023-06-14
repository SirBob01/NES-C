#include "./display.h"

display_t create_display(unsigned width, unsigned height, const char *title) {
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    display_t display;
    display.window = SDL_CreateWindow(title, 640, 480, SDL_WINDOW_RESIZABLE);
    if (!display.window) {
        fprintf(stderr,
                "Error: Unable to initialize window (%s)\n",
                SDL_GetError());
        exit(1);
    }

    unsigned flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    display.renderer = SDL_CreateRenderer(display.window, NULL, flags);
    if (!display.renderer) {
        fprintf(stderr,
                "Error: Unable to initialize display (%s)\n",
                SDL_GetError());
        exit(1);
    }

    display.texture = SDL_CreateTexture(display.renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        width,
                                        height);
    if (!display.texture) {
        fprintf(stderr,
                "Error: Unable to initialize texture (%s)\n",
                SDL_GetError());
        exit(1);
    }

    // Create the color buffer
    display.bitmap = allocate_memory(width * height * 4);
    display.size.x = width;
    display.size.y = height;

    // Set the logical presentation of the display
    SDL_SetRenderLogicalPresentation(display.renderer,
                                     width,
                                     height,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX,
                                     SDL_SCALEMODE_NEAREST);
    return display;
}

void destroy_display(display_t *display) {
    free_memory(&display->bitmap);
    SDL_DestroyTexture(display->texture);
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
}

void refresh_display(display_t *display) {
    // Update the texture
    SDL_UpdateTexture(display->texture,
                      NULL,
                      display->bitmap.buffer,
                      display->size.x * 4);

    // Render the texture
    SDL_SetRenderDrawBlendMode(display->renderer, SDL_BLENDMODE_NONE);
    SDL_RenderTexture(display->renderer, display->texture, NULL, NULL);

    // Present and clear
    SDL_RenderPresent(display->renderer);
    memset(display->bitmap.buffer, 0, display->bitmap.size);
}

void render_fill(display_t *display, color_t color) {
    unsigned *pixels = (unsigned *)display->bitmap.buffer;
    unsigned rgba = color.r << 24 | color.g << 16 | color.b << 8 | color.a;
    memset(pixels, rgba, display->size.x * display->size.y * 4);
}

void render_pixel(display_t *display, vec2_t position, color_t color) {
    unsigned *pixels = (unsigned *)display->bitmap.buffer;
    unsigned index = display->size.x * position.y + position.x;
    pixels[index] = color.r << 24 | color.g << 16 | color.b << 8 | color.a;
}