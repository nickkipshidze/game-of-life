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
