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
#ifndef _GRAPHIC_H
#define _GRAPHIC_H
#include "color.h"
using namespace std;
enum graphic_type
{
    TEXTURE,
    RECTANGLE,
    LINE,
    CIRCLE,
    SOLID_ARC
};
class graphic
{
    public:
    graphic_type type;
    graphic(graphic_type type) : type(type){}
    virtual ~graphic()
    {
        
    }
};
#endif