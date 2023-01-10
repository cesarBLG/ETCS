/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
