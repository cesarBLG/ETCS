/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _VOLUME_H
#define _VOLUME_H
#include "data_entry.h"
class volume_window : public input_window
{
    public:
    volume_window();
    void setLayout() override;
    void sendInformation() override;
};
#endif