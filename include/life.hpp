#pragma once

#include <vector>
#include <SDL3/SDL.h>

class Life {
    private:
        SDL_Vertex *vertices;
        int *indices;

        std::vector<Uint8> buffer1;
        std::vector<Uint8> buffer2;

        std::vector<int> nindices;

    public:
        std::vector<Uint8> *front;
        std::vector<Uint8> *back;

        SDL_FPoint offset = {0, 0};
        float cellsize = 16.0f;
        int width;
        int height;

        Life(int width, int height);
        
        ~Life();
        
        void clear(Uint8 value, std::vector<Uint8> &buffer);
        
        void swapbuffers();
        
        int toi(int x, int y);
        
        void cachenindices();

        SDL_Point getmouse();

        void appendRect(SDL_Vertex *vertices, int *indices, SDL_FRect rect, SDL_FColor color, int count=0);

        void renderGrid(SDL_Renderer *renderer);
        
        void render(SDL_Renderer *renderer);

        void step();
};
