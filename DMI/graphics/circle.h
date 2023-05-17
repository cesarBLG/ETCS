/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _CIRCLE_H
#define _CIRCLE_H
#include "graphic.h"
class circle : public graphic
{
    public:
    float cx;
    float cy;
    float radius;
    circle(float cx, float cy, float radius) : graphic(CIRCLE), cx(cx), cy(cy), radius(radius) {}
};
#endif