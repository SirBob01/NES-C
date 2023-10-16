#include "./nes.h"

void print_usage() { printf("Usage: nesc %s <input_file>\n", ARG_INPUT_FILE); }

void parse_args(emulator_t *emu, int argc, char **argv) {
    // Verify arguments
    if (argc < 3) {
        print_usage();
        exit(1);
    }
    if (strcmp(argv[1], ARG_INPUT_FILE) != 0) {
        print_usage();
        exit(1);
    }

    create_emulator(emu, argv[2]);
    if (argc == 4) {
        emu->cpu.pc = strtol(argv[3], NULL, 16);
    }
}

int main(int argc, char **argv) {
    // Initialize string buffer for debugging
    char strbuf[1024];

    // Boot up the emulator
    emulator_t emu;
    parse_args(&emu, argc, argv);
    if (emu.rom.data.buffer == NULL || emu.rom.header.type == NES_INVALID) {
        exit(1);
    }

    // Setup host machine IO
    io_t io;
    create_io(&io, &emu);

    // Print ROM information
    read_state_rom(&emu.rom, strbuf, sizeof(strbuf));
    puts(strbuf);

    // Emulate and refresh device IO every frame
    while (true) {
        bool emu_state = true;
        while (emu_state) {
            unsigned prev_frame = emu.frames;
            emu_state = update_emulator(&emu);
            unsigned curr_frame = emu.frames;

            if (curr_frame > prev_frame) {
                break;
            }
        }

        // Handle debug input
        if (is_keydown_input(&io.input, SDLK_o)) {
            set_debug_io(&io, true);
        }
        if (is_keydown_input(&io.input, SDLK_p)) {
            set_debug_io(&io, false);
        }

        // Handle controller 1 input
        joypad_t buttons;
        buttons.a = is_keydown_input(&io.input, SDLK_z);
        buttons.b = is_keydown_input(&io.input, SDLK_x);
        buttons.select = is_keydown_input(&io.input, SDLK_a);
        buttons.start = is_keydown_input(&io.input, SDLK_s);
        buttons.up = is_keydown_input(&io.input, SDLK_UP);
        buttons.down = is_keydown_input(&io.input, SDLK_DOWN);
        buttons.left = is_keydown_input(&io.input, SDLK_LEFT);
        buttons.right = is_keydown_input(&io.input, SDLK_RIGHT);

        set_joy1_controller(&emu.controller, buttons);

        // Refresh IO
        bool io_state = refresh_io(&io, &emu);
        if (!emu_state || !io_state) {
            break;
        }
    }

    // Cleanup
    destroy_io(&io);
    destroy_emulator(&emu);
    return 0;
}