/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
