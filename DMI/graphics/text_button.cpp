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
#include "text_button.h"
using namespace std;
TextButton::TextButton(string text, float sx, float sy, function<void()> pressed, int size) : size(size), caption(text)
{
    setSize(sx, sy);
    setPressedAction(pressed);
}
void TextButton::paint()
{
    if (enabled_text == nullptr)
    {
        enabled_text = getText(caption, 0, 0, size, fgColor);
        disabled_text = getText(caption, 0, 0, size, DarkGrey);
    }
    Button::paint();
    if(enabled)
    {
        if(enabled_text->color != fgColor)
        {
            delete enabled_text;
            enabled_text = getText(caption, 0, 0, size, fgColor);
        }
        draw(enabled_text);
    }
    else draw(disabled_text);
}
TextButton::~TextButton()
{
    if(enabled_text!=nullptr) delete enabled_text;
    if(disabled_text!=nullptr) delete disabled_text;
}
