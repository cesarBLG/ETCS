/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "driver_id.h"
#include "window.h"
#include "algorithm"
#include "../monitor.h"
#include "menu_settings.h"
#include "keyboard.h"
#include "../tcp/server.h"
driver_window::driver_window(std::string title, bool show_trn) : input_window(title, 1, false), TRN(get_text("TRN"),82,50,nullptr,10), settings("symbols/Setting/SE_04.bmp",82,50), show_trn(show_trn)
{
    TRN.setPressedAction([this] 
    {
        write_command("TrainRunningNumber","");
    });
    settings.setPressedAction([this] 
    {
        write_command("navButton","settings");
    });
}
void driver_window::setLayout()
{
    input_window::setLayout();
    if (show_trn)
    {
        addToLayout(&TRN, new RelativeAlignment(nullptr, 334+142,400+15,0));
        addToLayout(&settings, new ConsecutiveAlignment(&TRN, RIGHT,0));
    }
}