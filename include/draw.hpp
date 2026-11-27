#pragma once

#include <string>
#include <SDL3/SDL.h>

#include "life.hpp"

namespace GDraw {
    extern int rotation;
    extern bool ghost;
    extern SDL_Renderer *renderer;

    void renderghost(Life &world, int x, int y);
    void cell(Life &world, int x, int y, Uint8 v=1);
    void pattern(Life &world, int w, int h, Uint8 *pattern, int x, int y, Uint8 v=1);

    void square(Life &world, int x, int y, Uint8 v=1);
    void glider(Life &world, int x, int y, Uint8 v=1);
    void lightship(Life &world, int x, int y, Uint8 v=1);
    void middleship(Life &world, int x, int y, Uint8 v=1);
    void heavyship(Life &world, int x, int y, Uint8 v=1);
    void pentadecathlon(Life &world, int x, int y, Uint8 v=1);
    void galaxy(Life &world, int x, int y, Uint8 v=1);
    void glidergun(Life &world, int x, int y, Uint8 v=1);
};

namespace Brush {
    extern std::vector<void (*)(Life&, int, int, Uint8)> func;
    extern std::vector<std::string> label;
    extern int active;

    void draw(Life &world, int x, int y, Uint8 v=1);
    void renderghost(SDL_Renderer *renderer, Life &world, int x, int y, Uint8 v=1);
    std::string getlabel();
    void next(int offset=1);
}
