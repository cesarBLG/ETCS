/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "train_data.h"
#include "fixed_train_data.h"
#include "keyboard.h"
#include "../tcp/server.h"
#include "../monitor.h"
using json = nlohmann::json;
fixed_train_data_window::fixed_train_data_window(std::string title, bool switchable) : input_window(title, 1, true), SelectType(softkeys ? 64 : 60,50), softSelectType(get_text("Enter\ndata"), 64, 50), switchable(switchable)
{
    if (softkeys)
    {
        SelectType.addText(get_text("Enter\ndata"), 0, 0, 12, Black);
        SelectType.setBackgroundColor(MediumGrey);
        SelectType.showBorder = false;
    }
    else
    {
        SelectType.addText(get_text("Enter\ndata"));
    }
    SelectType.setPressedAction([this]() {
        write_command("EnterData", "");
    });
    softSelectType.setPressedAction([this]() {
        write_command("EnterData", "");
    });
}
void fixed_train_data_window::setLayout()
{
    input_window::setLayout();
    if (switchable)
    {
        if (softkeys)
        {
            addToLayout(&SelectType, new RelativeAlignment(nullptr, 334+178,380,0));
            if (cursor == -1) addToLayout(&softSelectType, new ConsecutiveAlignment(&SelectType, DOWN, 0));
        }
        else addToLayout(&SelectType, new RelativeAlignment(&exit_button, 246+30,25,0));
    }
}