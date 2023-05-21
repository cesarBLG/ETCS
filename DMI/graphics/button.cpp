/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "button.h"
void Button::paint()
{
    Component::paint();
    if(showBorder)
    {
        drawLine(0, 0, 0, sy - 1, Black);
        drawLine(sx - 1, 0, sx - 1, sy - 1, Shadow);
        drawLine(0, 0, sx - 1, 0, Black);
        drawLine(0, sy - 1, sx - 1, sy - 1, Shadow);

        if (!pressed)
        {
            drawLine(1, 1, 1, sy-2, Shadow);
            drawLine(sx-2, 1, sx-2, sy-2, Black);
            drawLine(1, 1, sx-2, 1, Shadow);
            drawLine(1, sy-2, sx-2, sy-2, Black);
        }
    }
}