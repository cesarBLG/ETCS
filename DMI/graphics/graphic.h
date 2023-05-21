/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _GRAPHIC_H
#define _GRAPHIC_H
#include "color.h"
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
    virtual ~graphic() = default;
};
#endif
