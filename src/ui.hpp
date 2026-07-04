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

struct DisplayWindowState {
    const char* name;
    ImTextureID display;
    float aspect;
};

struct InterpreterWindowState {
    const char* name;
    const char* program;
    const class Chip8& chip8;
};

void ImGui_Chip8_MainMenuBar(const MainMenuState& data);
void ImGui_Chip8_SetupLayout();
void ImGui_Chip8_DisplayWindow(const DisplayWindowState& state);
void ImGui_Chip8_InterpreterWindow(const InterpreterWindowState& state);
