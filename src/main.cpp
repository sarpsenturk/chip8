#include "chip8.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

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

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Chip8",
        Chip8::display_width * scale,
        Chip8::display_height * scale,
        0);

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    // Create chip8 framebuffer
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        Chip8::display_width,
        Chip8::display_height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

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
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        last = now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    goto exit;
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                    switch (event.key.key) {
                        case SDLK_X:
                            chip8.keys()[0] = event.key.down;
                            break;
                        case SDLK_1:
                            chip8.keys()[1] = event.key.down;
                            break;
                        case SDLK_2:
                            chip8.keys()[2] = event.key.down;
                            break;
                        case SDLK_3:
                            chip8.keys()[3] = event.key.down;
                            break;
                        case SDLK_Q:
                            chip8.keys()[4] = event.key.down;
                            break;
                        case SDLK_W:
                            chip8.keys()[5] = event.key.down;
                            break;
                        case SDLK_E:
                            chip8.keys()[6] = event.key.down;
                            break;
                        case SDLK_A:
                            chip8.keys()[7] = event.key.down;
                            break;
                        case SDLK_S:
                            chip8.keys()[8] = event.key.down;
                            break;
                        case SDLK_D:
                            chip8.keys()[9] = event.key.down;
                            break;
                        case SDLK_Z:
                            chip8.keys()[0xa] = event.key.down;
                            break;
                        case SDLK_C:
                            chip8.keys()[0xb] = event.key.down;
                            break;
                        case SDLK_4:
                            chip8.keys()[0xc] = event.key.down;
                            break;
                        case SDLK_R:
                            chip8.keys()[0xd] = event.key.down;
                            break;
                        case SDLK_F:
                            chip8.keys()[0xe] = event.key.down;
                            break;
                        case SDLK_V:
                            chip8.keys()[0xf] = event.key.down;
                            break;
                    }
            }
        }

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

        SDL_UpdateTexture(texture, nullptr, chip8.pixels(), Chip8::display_pitch);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // Destroy window & shutdown SDL
exit:
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
