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
    if(/*enabled&&*/showBorder)
    {
        drawLine(2, 2, 2, sy-3, Shadow);
        drawLine(sx-3, 2, sx-3, sy-3, Black);
        drawLine(2, 2, sx-3, 2, Shadow);
        drawLine(2, sy-3, sx-3, sy-3, Black);
    }
}