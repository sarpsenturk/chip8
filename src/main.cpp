#include "chip8.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include <string>

static constexpr auto cycles_per_frame = 10;
static constexpr auto keymap(SDL_Keycode keycode)
{
    switch (keycode) {
        case SDLK_X:
            return 0;
        case SDLK_1:
            return 1;
        case SDLK_2:
            return 2;
        case SDLK_3:
            return 3;
        case SDLK_Q:
            return 4;
        case SDLK_W:
            return 5;
        case SDLK_E:
            return 6;
        case SDLK_A:
            return 7;
        case SDLK_S:
            return 8;
        case SDLK_D:
            return 9;
        case SDLK_Z:
            return 10;
        case SDLK_C:
            return 11;
        case SDLK_4:
            return 12;
        case SDLK_R:
            return 13;
        case SDLK_F:
            return 14;
        case SDLK_V:
            return 15;
        default:
            return -1;
    }
}

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
    SDL_SetRenderVSync(renderer, 1);

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
                case SDL_EVENT_KEY_DOWN: {
                    if (const auto key = keymap(event.key.key); key != -1) {
                        chip8.set_key_down(static_cast<std::uint8_t>(key));
                        SDL_Log("Keydown = %i\n", key);
                    }
                } break;
                case SDL_EVENT_KEY_UP: {
                    if (const auto key = keymap(event.key.key); key != -1) {
                        chip8.set_key_up(static_cast<std::uint8_t>(key));
                        SDL_Log("Keyup = %i\n", key);
                    }
                } break;
            }
        }

        for (int i = 0; i < cycles_per_frame; ++i) {
            chip8.cycle();
            if (chip8.draw_flag()) {
                break;
            }
        }
        chip8.tick_timers();

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
