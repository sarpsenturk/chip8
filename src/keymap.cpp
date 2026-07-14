#include "keymap.hpp"

int keymap(SDL_Keycode keycode)
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
