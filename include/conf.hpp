#pragma once

#include <SDL3/SDL.h>

namespace conf {
    inline SDL_FPoint windowsize = {1080, 675};
    inline SDL_FPoint resolution = {1080, 675};

    inline SDL_Point gridsize = {2048, 1024};
    
    inline bool vsync = true;
    inline bool simulation = false;
    inline bool rendering = true;

    inline int steps = 1;
};

namespace conf::color {
    inline SDL_Color mainbg = {16, 16, 16, 255};
    inline SDL_Color gridbg = {4, 4, 4, 255};
    inline SDL_Color glines = {32, 32, 32, 255}; 
    inline SDL_FColor cell = {1, 1, 1, 1};
}
