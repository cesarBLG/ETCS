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
#ifndef _TEXT_BUTTON_H
#define _TEXT_BUTTON_H
#include "button.h"
class TextButton : public Button
{
    text_graphic *enabled_text=nullptr;
    text_graphic *disabled_text=nullptr;
    int size;
    public:
    std::string caption;
    Color disabledColor = DarkGrey;
    void paint() override;
    void rename(std::string name)
    {
        caption = name;
        enabled_text = disabled_text = nullptr;
    }
    TextButton(std::string text, float sx, float sy, std::function<void()> pressed = nullptr, int size = 12);
    ~TextButton();
};
#endif
