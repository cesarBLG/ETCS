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
#include <fstream>
using json = nlohmann::json;
fixed_train_data_window::fixed_train_data_window(std::string title, bool switchable) : input_window(title, 1, true), SelectType(get_text("Enter\ndata"),60,50), switchable(switchable)
{
    SelectType.setPressedAction([this]() {
        write_command("EnterData", "");
    });
}
void fixed_train_data_window::setLayout()
{
    input_window::setLayout();
    if (switchable) addToLayout(&SelectType, new RelativeAlignment(&exit_button, 246+30,25,0));
}