#pragma once

#include <imgui.h>

void ImGui_Chip8_MainMenuBar();
void ImGui_Chip8_SetupLayout();
void ImGui_Chip8_DisplayWindow(const char* name, ImTextureID display, float aspect);
void ImGui_Chip8_StateWindow(const char* name, const class Chip8& chip8);
