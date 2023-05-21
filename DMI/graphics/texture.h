/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _TEXTURE_H
#define _TEXTURE_H
#include "graphic.h"
#include <SDL.h>
#include <functional>
#include <memory>
class sdl_texture
{
    public:
    SDL_Texture *tex;
    int width;
    int height;
    sdl_texture(SDL_Texture *tex) : tex(tex) {}
    ~sdl_texture()
    {
        if(tex!=nullptr)
        {
            SDL_DestroyTexture(tex);
            tex = nullptr;
        }
    }
};
class texture : public graphic
{
    public:
    std::shared_ptr<sdl_texture> tex;
    float width;
    float height;
    float x;
    float y;
    texture() : graphic(TEXTURE){};
    texture(std::shared_ptr<sdl_texture> tex, float sx, float sy, float x, float y) : graphic(TEXTURE), tex(tex), width(sx), height(sy), x(x), y(y) {}
};
#endif
