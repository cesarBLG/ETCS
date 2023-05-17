/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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