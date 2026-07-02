#include "chip8.hpp"
#include "platform.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>

#include <string>

int main(int argc, const char** argv)
{
    // Parse command line arguments
    if (argc != 3) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Usage: %s [scale] [ROM]\n", argv[0]);
        return 1;
    }
    const auto scale = std::stoi(argv[1]);
    const char* filename = argv[2];

    // Initialize SDL and platform layer
    Platform platform("Chip8", Chip8::display_width * scale, Chip8::display_height * scale);

    // Initialize interpreter
    Chip8 chip8;

    // Main loop
    while (!platform.should_quit()) {
        platform.update(chip8.keys());
        chip8.cycle();
        platform.render(chip8.pixels(), Chip8::display_pitch);
    }
    return 0;
}
