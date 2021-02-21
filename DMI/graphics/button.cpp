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