#include "chip8.hpp"
#include "platform.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>

#include <chrono>
#include <string>

int main(int argc, const char** argv)
{
    // Parse command line arguments
    if (argc != 3) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Usage: %s [scale] [ROM]\n", argv[0]);
        return 1;
    }
    const auto scale = std::stoi(argv[1]);
    const char* rom = argv[2];

    // Initialize SDL and platform layer
    Platform platform("Chip8", Chip8::display_width, Chip8::display_height, scale);

    // Initialize interpreter
    Chip8 chip8;
    if (!chip8.load_rom(rom)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open ROM: %s\n", rom);
        return 1;
    }

    // Main loop
    auto last_time = std::chrono::high_resolution_clock::now();
    while (!platform.should_quit()) {
        platform.update(chip8.keys());

        const auto now = std::chrono::high_resolution_clock::now();
        const auto dt = std::chrono::duration<float, std::chrono::milliseconds::period>(now - last_time);
        if (dt.count() > 3) {
            last_time = now;
            chip8.cycle();
            platform.render(chip8.pixels(), Chip8::display_pitch);
        }
    }
    return 0;
}
