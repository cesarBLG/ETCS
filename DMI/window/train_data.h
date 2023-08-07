/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _TRAIN_DATA_WINDOW_H
#define _TRAIN_DATA_WINDOW_H
#include "data_entry.h"
#include "data_validation.h"
class train_data_window : public input_window
{
    bool switchable;
    public:
    Button SelectType;
    TextButton softSelectType;
    train_data_window(std::string title, bool switchable);
    void setLayout() override;
};
#endif