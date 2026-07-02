#include "chip8.hpp"
#include "platform.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>

#include <chrono>
#include <string>

static constexpr auto cpu_hz = 500.0;
static constexpr auto timer_hz = 60.0;

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
    double cpu_accumulator = 0.0;
    double timer_accumulator = 0.0;
    auto last = std::chrono::high_resolution_clock::now();
    while (!platform.should_quit()) {
        auto now = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        last = now;

        platform.update(chip8.keys());

        cpu_accumulator += dt * cpu_hz;
        timer_accumulator += dt * timer_hz;

        while (cpu_accumulator >= 1.0) {
            chip8.cycle();
            cpu_accumulator -= 1.0;
        }
        while (timer_accumulator >= 1.0) {
            chip8.tick_timers();
            timer_accumulator -= 1.0;
        }

        platform.render(chip8.pixels(), chip8.display_pitch);
    }

    return 0;
}
