/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _BRIGHTNESS_H
#define _BRIGHTNESS_H
#include "data_entry.h"
class brightness_window : public input_window
{
    public:
    brightness_window();
    void setLayout() override;
    void sendInformation() override;
};
#endif