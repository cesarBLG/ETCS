/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "icon_button.h"
using namespace std;
IconButton::IconButton(string enabled_path, float sx, float sy, function<void()> pressed, string disabled_path) : enabled_path(enabled_path), disabled_path(disabled_path)
{
    setSize(sx, sy);
    setPressedAction(pressed);
}
void IconButton::setEnabledImage(string path)
{
    if(path=="") return;
    enabled_path = path;
    enabled_image = getImage(path);
}
void IconButton::setDisabledImage(string path)
{
    if(path=="") return;
    disabled_path = path;
    disabled_image = getImage(path);
}
void IconButton::paint()
{
    if (enabled_image == nullptr)
    {
        setEnabledImage(enabled_path);
        setDisabledImage(disabled_path);
    }
    Button::paint();
    if(enabled || disabled_image == nullptr) draw(enabled_image);
    else draw(disabled_image);
}
IconButton::~IconButton()
{
    if(enabled_image != nullptr) delete enabled_image;
    if(disabled_image != nullptr) delete disabled_image;
}
