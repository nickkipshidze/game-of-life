#include <format>
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "conf.hpp"
#include "life.hpp"
#include "draw.hpp"

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

void renderVignette(SDL_Renderer *renderer, SDL_FColor color={1, 1, 1, 1}) {
    float w = conf::resolution.x, h = conf::resolution.y;
    SDL_Vertex vertices[5] = {
        {{0, 0}, color, {0, 0}}, // Top left
        {{w, 0}, color, {0, 0}}, // Top right
        {{0, h}, color, {0, 0}}, // Bottom left
        {{w, h}, color, {0, 0}}, // Bottom right
        {{w/2, h/2}, {0, 0, 0, 0}, {0, 0}}, // Center
    };
    int indices[12] = {
        0, 1, 4,
        1, 3, 4,
        3, 2, 4,
        2, 0, 4
    };
    SDL_RenderGeometry(renderer, nullptr, vertices, 5, indices, 12);
}

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
// Continued dev 9:15 AM
// Paused dev 10:46 AM
// Continued dev 11:05 AM
// Paused dev 1:18 PM
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
    GDraw::glider(world, center.x+32, center.y+19);

    const bool *numkeys = SDL_GetKeyboardState(NULL);

    Uint64 clockNow = SDL_GetPerformanceCounter(), clockLast = clockNow;
    FPS fps;

    bool running = true;
    while (running) {
        clockNow = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>(clockNow - clockLast) / SDL_GetPerformanceFrequency();
        deltaTime = SDL_clamp(deltaTime, 0.0001f, 1.0f);
        clockLast = clockNow;

        SDL_Point mousePos = world.getmouse();

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
                    mouseX = (mouseX / conf::windowsize.x) * conf::resolution.x;
                    mouseY = (mouseY / conf::windowsize.y) * conf::resolution.y;

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
                case SDL_EVENT_MOUSE_MOTION: {
                    SDL_MouseButtonFlags mouseFlags = SDL_GetMouseState(nullptr, nullptr);
                    if (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) Brush::draw(world, mousePos.x, mousePos.y, 1);
                    if (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) Brush::draw(world, mousePos.x, mousePos.y, 0);
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    if (event.button.button == SDL_BUTTON_LEFT) Brush::draw(world, mousePos.x, mousePos.y, 1);
                    if (event.button.button == SDL_BUTTON_RIGHT) Brush::draw(world, mousePos.x, mousePos.y, 0);
                    break;
                }
                case SDL_EVENT_KEY_DOWN: {
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                        running = false;
                    if (event.key.scancode == SDL_SCANCODE_V) {
                        conf::vsync = !conf::vsync;
                        SDL_SetRenderVSync(renderer, conf::vsync);
                    }
                    if (event.key.scancode == SDL_SCANCODE_SPACE)
                        conf::simulation = !conf::simulation;
                    if (event.key.scancode == SDL_SCANCODE_RETURN)
                        world.step();
                    if (event.key.scancode == SDL_SCANCODE_P)
                        conf::rendering = !conf::rendering;
                    if (event.key.scancode == SDL_SCANCODE_E)
                        Brush::next(1);
                    if (event.key.scancode == SDL_SCANCODE_Q)
                        Brush::next(-1);
                    if (event.key.scancode == SDL_SCANCODE_R) {
                        GDraw::rotation += 1;
                        GDraw::rotation %= 4;
                    }
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

        float moveSpeedup = 32.0f;
        if (numkeys[SDL_SCANCODE_LSHIFT]) moveSpeedup = 100.0f;

        if (numkeys[SDL_SCANCODE_W]) camera.offsetTarget.y += camera.speed * moveSpeedup * deltaTime;
        if (numkeys[SDL_SCANCODE_A]) camera.offsetTarget.x += camera.speed * moveSpeedup * deltaTime;
        if (numkeys[SDL_SCANCODE_S]) camera.offsetTarget.y -= camera.speed * moveSpeedup * deltaTime;
        if (numkeys[SDL_SCANCODE_D]) camera.offsetTarget.x -= camera.speed * moveSpeedup * deltaTime;

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

        SDL_SetRenderDrawColor(renderer, conf::color::mainbg.r, conf::color::mainbg.g, conf::color::mainbg.b, conf::color::mainbg.a);
        SDL_RenderClear(renderer);

        if (conf::simulation) {
            for (int step{0}; step < conf::steps; ++step)
                world.step();
        }
        if (conf::rendering) {
            world.render(renderer);
        }

        Brush::renderghost(renderer, world, mousePos.x, mousePos.y);
        renderVignette(renderer, {0, 0, 0, 0.4});

        std::vector<std::string> info = {
            std::format("{} - Nick Kipshidze", __DATE__),
            std::format("FPS {}", SDL_roundf(fps.avg)),
            std::format("Grid {}x{}", conf::gridsize.x, conf::gridsize.y),
            std::format("VSync (V) {}", conf::vsync),
            std::format("Render (P) {}", conf::rendering),
            std::format("Sim (Space/Enter) {}", conf::simulation),
            std::format("Selected [{}, {}]", mousePos.x, mousePos.y),
            std::format("Brush (Q/E/R) \"{}\"", Brush::getlabel())
        };
        for (int message{0}; message < (int)info.size(); ++message) {
            renderText(
                renderer, fontPixelifyRegular, info[message].c_str(),
                {10.0f, 10.0f+message*30.0f}, {255, 255, 255, 255}
            );
        }

        SDL_RenderPresent(renderer);

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
    }

    TTF_CloseFont(fontPixelifyRegular);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
