/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _IMAGE_GRAPHIC_H
#define _IMAGE_GRAPHIC_H
#include "texture.h"
#include <string>
class image_graphic : public texture
{
    public:
    std::string path;
    float sx;
    float sy;
    float cx;
    float cy;
};
#endif
