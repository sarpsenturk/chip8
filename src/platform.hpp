#pragma once

#include <SDL3/SDL.h>

#include <SDL3/SDL_render.h>
#include <cstdint>

class Platform
{
public:
    Platform(const char* title, int width, int height, int scale);
    ~Platform();

    void update(std::uint8_t* keys);
    void render(void* buffer, int pitch);

    [[nodiscard]] bool should_quit() const noexcept { return quit_; }

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;

    bool quit_ = false;
};
