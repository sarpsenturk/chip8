#pragma once

#include <SDL3/SDL_keycode.h>

// Convert SDL_Keycode to the mapped Chip8 keypad key
int keymap(SDL_Keycode keycode);
