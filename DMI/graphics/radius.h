/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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