#include <vector>
#include <iostream>
#include <SDL3/SDL.h>

namespace conf {
    SDL_FPoint windowsize = {1080, 675};
    SDL_FPoint resolution = {1080, 675};
};

class Life {
    private:
        SDL_Vertex *vertices;
        int *indices;

        std::vector<u_int8_t> buffer1;
        std::vector<u_int8_t> buffer2;

        std::vector<int> nindices;

    public:
        std::vector<u_int8_t> *front;
        std::vector<u_int8_t> *back;

        SDL_FPoint offset = {0, 0};
        float cellsize = 16.0f;
        int width;
        int height;

        Life(int width, int height) : width(width), height(height) {
            this->front = &this->buffer1;
            this->back = &this->buffer2;

            this->front->reserve(width*height);
            this->back->reserve(width*height);

            this->clear(0, *this->front);
            this->clear(0, *this->back);

            this->vertices = new SDL_Vertex[this->width*this->height*4];
            this->indices = new int[this->width*this->height*6];

            this->cachenindices();
        };

        ~Life() {
            this->buffer1.clear();
            this->buffer2.clear();
            this->front = nullptr;
            this->back = nullptr;
            delete[] vertices;
            delete[] indices;
        }

        void clear(u_int8_t value, std::vector<u_int8_t> &buffer) {
            while ((int)buffer.size() < this->width*this->height)
                buffer.push_back(0);
            for (int i{0}; i<this->width*this->height; i++)
                buffer[i] = value;
        }

        void swapbuffers() {
            std::vector<u_int8_t> *temp = back;
            back = front;
            front = temp;
            temp = nullptr;
        }

        int toi(int x, int y) {
            if (x >= this->width) x = x-this->width;
            if (y >= this->height) y = y-this->height;
            if (x < 0) x += this->width;
            if (y < 0) y += this->height;
            return (y*this->width)+x;
        }

        void cachenindices() {
            for (int y{0}; y < this->height; ++y) {
                for (int x{0}; x < this->width; ++x) {
                    this->nindices.push_back(toi(x-1, y-1));
                    this->nindices.push_back(toi(x-0, y-1));
                    this->nindices.push_back(toi(x+1, y-1));
                    this->nindices.push_back(toi(x-1, y));
                    this->nindices.push_back(toi(x+1, y));
                    this->nindices.push_back(toi(x-1, y+1));
                    this->nindices.push_back(toi(x-0, y+1));
                    this->nindices.push_back(toi(x+1, y+1));
                }
            }
        }

        SDL_Point getmouse() {
            SDL_FPoint mousePos;
            SDL_GetMouseState(&mousePos.x, &mousePos.y);
            mousePos.x = (mousePos.x / conf::windowsize.x) * conf::resolution.x - this->offset.x;
            mousePos.y = (mousePos.y / conf::windowsize.y) * conf::resolution.y - this->offset.y;

            SDL_Point gridPos = {
                static_cast<int>(mousePos.x / (this->cellsize)),
                static_cast<int>(mousePos.y / (this->cellsize))
            };

            return gridPos;
        }

        void appendRect(SDL_Vertex *vertices, int *indices, SDL_FRect rect, SDL_FColor color, int count=0) {
            int offsetv = count*4;
            int offseti = count*6;
            vertices[offsetv+0] = {{rect.x+rect.w, rect.y}, color, {}};
            vertices[offsetv+1] = {{rect.x, rect.y}, color, {}};
            vertices[offsetv+2] = {{rect.x, rect.y+rect.h}, color, {}};
            vertices[offsetv+3] = {{rect.x+rect.w, rect.y+rect.h}, color, {}};
            indices[offseti+0] = offsetv+0;
            indices[offseti+1] = offsetv+1;
            indices[offseti+2] = offsetv+2;
            indices[offseti+3] = offsetv+0;
            indices[offseti+4] = offsetv+3;
            indices[offseti+5] = offsetv+2;
        }

        void render(SDL_Renderer *renderer) {
            float size = this->cellsize;
            float gaps = 1.0f;

            if (size < 5.0f) gaps = 0.0f;

            int index = 0;
            int rectangles = 0;
            for (int y{0}; y < this->height; ++y) {
                for (int x{0}; x < this->width; ++x) {
                    if ((*front)[index] == (*back)[index] && (*front)[index] == 0) {
                        index++;
                        continue;
                    }
                    SDL_FRect cell = {
                        this->offset.x+x*(this->cellsize),
                        this->offset.y+y*(this->cellsize),
                        this->cellsize-gaps,
                        this->cellsize-gaps
                    };                    
                    float value = (*front)[index]; index++;
                    appendRect(vertices, indices, cell, {value, value, value, 1}, rectangles);
                    rectangles++;
                }
            }

            SDL_FRect bgrect = {this->offset.x, this->offset.y, this->cellsize*this->width, this->cellsize*this->height};
            SDL_SetRenderDrawColor(renderer, 4, 4, 4, 255);
            SDL_RenderFillRect(renderer, &bgrect);
            SDL_RenderGeometry(renderer, nullptr, vertices, rectangles*4, indices, rectangles*6);
        }

        void step() {
            int index = 0;
            for (int y{0}; y < this->height; ++y) {
                for (int x{0}; x < this->width; ++x) {
                    int neighbors = (
                        (*front)[this->nindices[index*8+0]]+
                        (*front)[this->nindices[index*8+1]]+
                        (*front)[this->nindices[index*8+2]]+
                        (*front)[this->nindices[index*8+3]]+
                        (*front)[this->nindices[index*8+4]]+
                        (*front)[this->nindices[index*8+5]]+
                        (*front)[this->nindices[index*8+6]]+
                        (*front)[this->nindices[index*8+7]]
                    );
                    
                    if (neighbors < 2) (*back)[index] = 0;
                    else if (neighbors == 3) (*back)[index] = 1;
                    else if (neighbors > 3) (*back)[index] = 0;
                    else (*back)[index] = (*front)[index];

                    index++;
                }
            }
            this->swapbuffers();
        }
};

// Started dev 5:54 PM
// Paused dev 8:25 PM
int main(int, char *[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Conway's Game of Life", conf::windowsize.x, conf::windowsize.y, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);

    SDL_SetRenderLogicalPresentation(renderer, conf::resolution.x, conf::resolution.y, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderVSync(renderer, 1);

    Life world(2048, 1024);
    bool simrun = false;
    bool render = true;
    bool vsync = true;

    Uint64 clockNow = SDL_GetPerformanceCounter(), clockLast = clockNow;

    int fpsRaw = 0;
    int fpsSum = 0;
    int fpsCount = 0;
    float fpsCooldown = 1.0f;

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
                }
                case SDL_EVENT_MOUSE_WHEEL: {
                    float mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    world.cellsize *= 1.0f + event.wheel.y * 0.1f;
                    float oldSize = world.cellsize;
                    world.cellsize *= 1.0f + event.wheel.y * 0.1f;
                    float zoomRatio = world.cellsize / oldSize;
                    world.offset.x = mouseX - (mouseX - world.offset.x) * zoomRatio;
                    world.offset.y = mouseY - (mouseY - world.offset.y) * zoomRatio;
                }
                case SDL_EVENT_KEY_DOWN: {
                    if (event.key.scancode == SDL_SCANCODE_Q)
                        running = false;
                    if (event.key.scancode == SDL_SCANCODE_V) {
                        vsync = !vsync;
                        SDL_SetRenderVSync(renderer, vsync);
                    }
                    if (event.key.scancode == SDL_SCANCODE_SPACE)
                        simrun = !simrun;
                    if (event.key.scancode == SDL_SCANCODE_R)
                        render = !render;
                    break;
                }
            }

            if (event.type == SDL_EVENT_MOUSE_MOTION && (event.motion.state & SDL_BUTTON_MMASK)) {
                float dx = event.motion.xrel;
                float dy = event.motion.yrel;
                world.offset.x += dx;
                world.offset.y += dy;
            }
        }

        SDL_SetRenderDrawColor(renderer, 8, 8, 8, 255);
        SDL_RenderClear(renderer);

        SDL_MouseButtonFlags mouseFlags = SDL_GetMouseState(nullptr, nullptr);
        if (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
            SDL_Point mousePos = world.getmouse();
            (*world.front)[world.toi(mousePos.x, mousePos.y)] = 1;
        } else if (mouseFlags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
            SDL_Point mousePos = world.getmouse();
            (*world.front)[world.toi(mousePos.x, mousePos.y)] = 0;
        }

        if (simrun) { world.step(); }
        if (render) world.render(renderer);

        fpsRaw = SDL_roundf(1.0f/deltaTime);
        fpsSum += fpsRaw;
        fpsCount++;

        fpsCooldown -= deltaTime;
        if (fpsCooldown <= 0) {
            fpsCooldown = 1.0f;
            float fpsAvg = (float)fpsSum / fpsCount;
            fpsSum = 0;
            fpsCount = 0;
            std::cout << "FPS / AVG : " << fpsRaw << "/" << fpsAvg << std::endl;
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
