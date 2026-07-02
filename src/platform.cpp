#include "platform.hpp"

#include <SDL3/SDL_events.h>

Platform::Platform(const char* title, int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetAppMetadata("Chip8", "0.1.0", "");

    window_ = SDL_CreateWindow(title, width, height, 0);
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

Platform::~Platform()
{
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Platform::update(std::uint8_t* keys)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                quit_ = true;
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                switch (event.key.key) {
                    case SDLK_X:
                        keys[0] = event.key.down;
                        break;
                    case SDLK_1:
                        keys[1] = event.key.down;
                        break;
                    case SDLK_2:
                        keys[2] = event.key.down;
                        break;
                    case SDLK_3:
                        keys[3] = event.key.down;
                        break;
                    case SDLK_Q:
                        keys[4] = event.key.down;
                        break;
                    case SDLK_W:
                        keys[5] = event.key.down;
                        break;
                    case SDLK_E:
                        keys[6] = event.key.down;
                        break;
                    case SDLK_A:
                        keys[7] = event.key.down;
                        break;
                    case SDLK_S:
                        keys[8] = event.key.down;
                        break;
                    case SDLK_D:
                        keys[9] = event.key.down;
                        break;
                    case SDLK_Z:
                        keys[0xa] = event.key.down;
                        break;
                    case SDLK_C:
                        keys[0xb] = event.key.down;
                        break;
                    case SDLK_4:
                        keys[0xc] = event.key.down;
                        break;
                    case SDLK_R:
                        keys[0xd] = event.key.down;
                        break;
                    case SDLK_F:
                        keys[0xe] = event.key.down;
                        break;
                    case SDLK_V:
                        keys[0xf] = event.key.down;
                        break;
                }
        }
    }
}

void Platform::render(void* buffer, int pitch)
{
    SDL_UpdateTexture(texture_, nullptr, buffer, pitch);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}
