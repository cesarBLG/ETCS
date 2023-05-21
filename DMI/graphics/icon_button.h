/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
