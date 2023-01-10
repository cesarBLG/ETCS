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
