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
#ifndef _ICON_BUTTON_H
#define _ICON_BUTTON_H
#include "button.h"
class IconButton : public Button
{
    image_graphic *enabled_image = nullptr;
    image_graphic *disabled_image = nullptr;
    std::string enabled_path;
    std::string disabled_path;
    public:
    void paint() override;
    void setEnabledImage(std::string path);
    void setDisabledImage(std::string path);
    IconButton(std::string enabled_path, float sx, float sy, std::function<void()> pressed = nullptr, std::string disabled_path = "");
    ~IconButton();
};
#endif
