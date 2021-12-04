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
#ifndef _BUTTON_H
#define _BUTTON_H
#include "component.h"
class Button : public Component
{
    protected:
    Button() {isButton = true;}
    public:
    bool enabled = true;
    bool pressed = false;
    bool showBorder = true;
    virtual void paint();
    virtual void setEnabled(bool val) {enabled = val;}
    Button(float sx, float sy, std::function<void()> display = nullptr, std::function<void()> pressed = nullptr) : Component(sx,sy,display)
    {
        isButton = true;
        setPressedAction(pressed);
    }
    virtual ~Button(){}
};
#endif
