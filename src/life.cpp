#include <SDL3/SDL.h>

#include "conf.hpp"
#include "life.hpp"

Life::Life(int width, int height) : width(width), height(height) {
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

Life::~Life() {
    this->buffer1.clear();
    this->buffer2.clear();
    this->front = nullptr;
    this->back = nullptr;
    delete[] vertices;
    delete[] indices;
}

void Life::clear(Uint8 value, std::vector<Uint8> &buffer) {
    while ((int)buffer.size() < this->width*this->height)
        buffer.push_back(0);
    for (int i{0}; i<this->width*this->height; i++)
        buffer[i] = value;
}

void Life::swapbuffers() {
    std::vector<Uint8> *temp = back;
    back = front;
    front = temp;
    temp = nullptr;
}

int Life::toi(int x, int y) {
    if (x >= this->width) x = x-this->width;
    if (y >= this->height) y = y-this->height;
    if (x < 0) x += this->width;
    if (y < 0) y += this->height;
    return (y*this->width)+x;
}

void Life::cachenindices() {
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

SDL_Point Life::getmouse() {
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

void Life::appendRect(SDL_Vertex *vertices, int *indices, SDL_FRect rect, SDL_FColor color, int count) {
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

void Life::renderGrid(SDL_Renderer *renderer) {
    float alpha = 1.0f-SDL_clamp((this->cellsize-18.0f)/-23.0f, 0.0f, 1.0f);
    SDL_SetRenderDrawColor(
        renderer,
        conf::color::glines.r*alpha,
        conf::color::glines.g*alpha,
        conf::color::glines.b*alpha,
        conf::color::glines.a*alpha
    );
    
    for (float y{this->offset.y}; y < this->offset.y+this->height*this->cellsize; y+=this->cellsize)
        SDL_RenderLine(renderer, 0, y, conf::resolution.x, y);
    for (float x{this->offset.x}; x < this->offset.x+this->width*this->cellsize; x+=this->cellsize)
        SDL_RenderLine(renderer, x, 0, x, conf::resolution.y);
}

void Life::render(SDL_Renderer *renderer) {
    float gaps = 1.0f;
    if (this->cellsize < 3.0f) gaps = 0;

    int index = 0, rectangles = 0;
    for (int y{0}; y < this->height; ++y) {
        for (int x{0}; x < this->width; ++x) {
            float value = (*front)[index]; index++;
            if ((value == (*back)[index] && value == 0) || value == 0) {
                continue;
            }
            SDL_FRect cell = {
                this->offset.x+x*(this->cellsize),
                this->offset.y+y*(this->cellsize),
                this->cellsize-gaps,
                this->cellsize-gaps
            };
            appendRect(vertices, indices, cell, {
                conf::color::cell.r*value,
                conf::color::cell.g*value,
                conf::color::cell.b*value,
                conf::color::cell.a
            }, rectangles);
            rectangles++;
        }
    }

    SDL_FRect bgrect = {this->offset.x, this->offset.y, this->cellsize*this->width, this->cellsize*this->height};
    SDL_SetRenderDrawColor(
        renderer,
        conf::color::gridbg.r,
        conf::color::gridbg.g,
        conf::color::gridbg.b,
        conf::color::gridbg.a
    );
    SDL_RenderFillRect(renderer, &bgrect);
    
    if (this->cellsize > 2.0f) this->renderGrid(renderer);

    SDL_RenderGeometry(renderer, nullptr, vertices, rectangles*4, indices, rectangles*6);
}

void Life::step() {
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
