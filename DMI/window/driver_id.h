/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _DRIVER_ID_H
#define _DRIVER_ID_H
#include "data_entry.h"
class driver_window : public input_window
{
    TextButton TRN;
    IconButton settings;
    bool show_trn;
    public:
    driver_window(std::string title, bool show_trn);
    void setLayout() override;
};
#endif