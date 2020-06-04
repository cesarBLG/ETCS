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
#ifndef _LINE_H
#define _LINE_H
#include "graphic.h"
#include "color.h"
class line : public graphic
{
    protected:
    line() : graphic(LINE){}
    public:
    float x1;
    float y1;
    float x2;
    float y2;
    Color color;
    line(float x1, float y1, float x2, float y2, Color col) : graphic(LINE), x1(x1), y1(y1), x2(x2), y2(y2), color(col){}
};
#endif