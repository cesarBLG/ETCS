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
#include "window.h"
void window::event(int evNo, float x, float y)
{
    vector<LayoutElement>& el = getLayoutElements();
    for(int i=0; i<el.size(); i++)
    {
        Component *comp = el[i].comp;
        if((comp->x-comp->touch_left)<x && (comp->x + comp->sx + comp->touch_right)>x
        && (comp->y-comp->touch_up)<y && (comp->y + comp->sy+comp->touch_down)>y)
        {
            if(comp->isSensitive()) comp->setPressed(true);
        }
    }
}
void window::display()
{
    l->update();
}
window::~window()
{
    delete l;
}