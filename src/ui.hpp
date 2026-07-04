#pragma once

#include <SDL3/SDL.h>
#include <imgui.h>

struct MainMenuState {
    void (*file_dialog_callback)(void*, const char* const*, int);
    void* userdata;
    SDL_Window* window;
    const SDL_DialogFileFilter* filters;
    int nfilters;
};

void ImGui_Chip8_MainMenuBar(const MainMenuState& data);
void ImGui_Chip8_SetupLayout();
void ImGui_Chip8_DisplayWindow(const char* name, ImTextureID display, float aspect);
void ImGui_Chip8_StateWindow(const char* name, const class Chip8& chip8);
