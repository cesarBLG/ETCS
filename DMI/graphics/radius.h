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
#ifndef _RADIUS_H
#define _RADIUS_H
#include "line.h"
#include <cmath>
class radius : public line
{
    public:
    radius(float angle, float cx, float cy, float rmin, float rmax, Color col) : line()
    {
        float c = cosf(angle);
        float s = sinf(angle);
        x1 = rmin*c + cx;
        x2 = rmax*c + cx;
        y1 = rmin*s + cy;
        y2 = rmax*s + cy;
        color = col;
    }
};
#endif