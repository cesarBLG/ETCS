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
        enabled_text = getTextGraphic(caption, size, fgColor, 0, 0, sx);
        disabled_text = getTextGraphic(caption, size, disabledColor, 0, 0, sx);
    }
    Button::paint();
    drawTexture(enabled ? enabled_text : disabled_text, sx / 2, sy / 2);
}

TextButton::~TextButton()
{
    enabled_text = nullptr;
    disabled_text = nullptr;
}
