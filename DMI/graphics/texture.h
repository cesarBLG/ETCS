#ifndef _TEXTURE_H
#define _TEXTURE_H
#include "graphic.h"
#include <SDL2/SDL.h>
#include <functional>
#include <cstdio>
class texture : public graphic
{
    public:
    SDL_Texture *tex = nullptr;
    float width;
    float height;
    float x;
    float y;
    function<void()> load_function;
    virtual void load()
    {
        if(load_function!=nullptr) load_function();
    }
    texture() : graphic(TEXTURE){};
    texture(SDL_Texture *tex, float sx, float sy, float x, float y) : graphic(TEXTURE), tex(tex), width(sx), height(sy), x(x), y(y) {}
    ~texture()
    {
        if(tex!=nullptr) SDL_DestroyTexture(tex);
    }
};
#endif