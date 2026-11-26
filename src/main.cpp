#include <format>
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "conf.hpp"
#include "life.hpp"

struct FPS {
    int raw = 0;
    int sum = 0;
    int count = 0;
    float avg = 0;
    float cooldown = 0;
};

struct Viewport {
    SDL_FPoint offset = {0.0f, 0.0f};
    SDL_FPoint offsetTarget = {0.0f, 0.0f};
    float scaleTarget = 1.0f;
    float scale = 1.0f;
    float speed = 16.0f;
};

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_FPoint position, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, 0, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FRect dstrect = {position.x, position.y, (float)texture->w, (float)texture->h};
    SDL_RenderTexture(renderer, texture, nullptr, &dstrect);
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

// Started dev 5:54 PM
// Paused dev 8:25 PM
// Continued dev 9:15 PM
// Paused dev 11:50 PM
int main(int, char *[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Conway's Game of Life", conf::windowsize.x, conf::windowsize.y, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);

    SDL_SetRenderLogicalPresentation(renderer, conf::resolution.x, conf::resolution.y, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderVSync(renderer, 1);

    TTF_Font *fontPixelifyRegular = TTF_OpenFont("./assets/PixelifySans-Regular.ttf", 24);

    Life world(conf::gridsize.x, conf::gridsize.y);
    int cellsize = 16.0f;

    SDL_FPoint center = {conf::gridsize.x/2.0f, conf::gridsize.y/2.0f};
    Viewport camera{
        .offset={-center.x*cellsize, -center.y*cellsize},
        .offsetTarget={-center.x*cellsize, -center.y*cellsize}
    };

    // Starter glider (Also my profile picture)
    (*world.front)[world.toi(center.x+32, center.y+21)] = 1;
    (*world.front)[world.toi(center.x+33, center.y+21)] = 1;
    (*world.front)[world.toi(center.x+34, center.y+21)] = 1;
    (*world.front)[world.toi(center.x+34, center.y+20)] = 1;
    (*world.front)[world.toi(center.x+33, center.y+19)] = 1;

    Uint64 clockNow = SDL_GetPerformanceCounter(), clockLast = clockNow;
    FPS fps;

    bool running = true;
    while (running) {
        clockNow = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>(clockNow - clockLast) / SDL_GetPerformanceFrequency();
        deltaTime = SDL_clamp(deltaTime, 0.0001f, 1.0f);
        clockLast = clockNow;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    running = false;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED: {
                    conf::windowsize.x = event.window.data1;
                    conf::windowsize.y = event.window.data2;
                    break;
                }
                case SDL_EVENT_MOUSE_WHEEL: {
                    float mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    float oldScale = camera.scaleTarget;
                    camera.scaleTarget = SDL_clamp(
                        camera.scaleTarget * SDL_powf(1.1f, event.wheel.y),
                        0.05f, 10.0f
                    );
                    float ratio = camera.scaleTarget / oldScale;
                    camera.offsetTarget.x = mouseX - (mouseX - camera.offsetTarget.x) * ratio;
                    camera.offsetTarget.y = mouseY - (mouseY - camera.offsetTarget.y) * ratio;
                    break;
                }
                case SDL_EVENT_KEY_DOWN: {
                    if (event.key.scancode == SDL_SCANCODE_Q)
                        running = false;
                    if (event.key.scancode == SDL_SCANCODE_V) {
                        conf::vsync = !conf::vsync;
                        SDL_SetRenderVSync(renderer, conf::vsync);
                    }
                    if (event.key.scancode == SDL_SCANCODE_SPACE)
                        conf::simulation = !conf::simulation;
                    if (event.key.scancode == SDL_SCANCODE_R)
                        conf::rendering = !conf::rendering;
                    break;
                }
            }

            if (event.type == SDL_EVENT_MOUSE_MOTION && (event.motion.state & SDL_BUTTON_MMASK)) {
                float dx = event.motion.xrel;
                float dy = event.motion.yrel;
                camera.offsetTarget.x += dx;
                camera.offsetTarget.y += dy;
            }
        }

        SDL_SetRenderDrawColor(renderer, 8, 8, 8, 255);
        SDL_RenderClear(renderer);

        SDL_MouseButtonFlags mouseFlags = SDL_GetMouseState(nullptr, nullptr);
        SDL_Point mousePos = world.getmouse();
        if (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
            (*world.front)[world.toi(mousePos.x, mousePos.y)] = 1;
        } else if (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
            (*world.front)[world.toi(mousePos.x, mousePos.y)] = 0;
        }

        SDL_FPoint diff = {
            camera.offsetTarget.x - camera.offset.x,
            camera.offsetTarget.y - camera.offset.y
        };
        float moveT = 1.0f - SDL_expf(-camera.speed * deltaTime);
        camera.offset.x += diff.x * moveT;
        camera.offset.y += diff.y * moveT;
        float zoomT = 1.0f - SDL_expf(-camera.speed * deltaTime);
        camera.scale += (camera.scaleTarget - camera.scale) * zoomT;
        world.offset = camera.offset;
        world.cellsize = cellsize * camera.scale;

        if (conf::simulation) {
            for (int step{0}; step < conf::steps; ++step)
                world.step();
        }
        if (conf::rendering) {
            world.render(renderer);
        }

        std::vector<std::string> info = {
            std::format("{} - Nick Kipshidze", __DATE__),
            std::format("FPS {}", SDL_roundf(fps.avg)),
            std::format("Grid {}x{}", conf::gridsize.x, conf::gridsize.y),
            std::format("VSync (V) {}", conf::vsync),
            std::format("Render (R) {}", conf::rendering),
            std::format("Sim (Space) {}", conf::simulation),
            std::format("Selected [{}, {}]", mousePos.x, mousePos.y),
        };
        for (int message{0}; message < (int)info.size(); ++message) {
            renderText(
                renderer, fontPixelifyRegular, info[message].c_str(),
                {10.0f, 10.0f+message*30.0f}, {255, 255, 255, 255}
            );
        }

        fps.raw = SDL_roundf(1.0f/deltaTime);
        fps.sum += fps.raw;
        fps.count++;
        fps.cooldown -= deltaTime;
        if (fps.cooldown <= 0) {
            fps.cooldown = 1.0f;
            fps.avg = (float)fps.sum / fps.count;
            fps.sum = 0;
            fps.count = 0;
        }

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(fontPixelifyRegular);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
