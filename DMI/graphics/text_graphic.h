/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _TEXT_GRAPHIC_H
#define _TEXT_GRAPHIC_H
#include "texture.h"
#include <string>
class text_graphic : public texture
{
    public:
    std::string text;
    float size;
    float offx;
    float offy;
    int alignment;
    int aspect;
    Color color;
};
#endif
