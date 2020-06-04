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
#ifndef _RECTANGLE_H
#define _RECTANGLE_H
#include "graphic.h"
#include "color.h"
class rectangle : public graphic
{
    public:
    float sx;
    float sy;
    float x;
    float y;
    Color color;
    rectangle(float x, float y, float sx, float sy, Color col) : graphic(RECTANGLE), sx(sx), sy(sy), x(x), y(y), color(col){}
};
#endif