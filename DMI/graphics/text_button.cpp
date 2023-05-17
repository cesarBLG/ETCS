/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
        disabled_text = getText(caption, 0, 0, size, disabledColor);
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
