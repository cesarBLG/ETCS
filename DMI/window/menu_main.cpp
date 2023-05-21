/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "menu_main.h"
#include "window.h"
#include "driver_id.h"
#include "fixed_train_data.h"
#include <algorithm>
#include "../monitor.h"
#include "../messages/messages.h"
#include "../time_etcs.h"
#include "../control/control.h"
#include "../tcp/server.h"

#include <chrono>
menu_main::menu_main() : menu(get_text("Main"))
{
    buttons[0] = new TextButton(get_text("Start"), 153, 50);
    buttons[1] = new TextButton(get_text("Driver ID"), 153, 50);
    buttons[2] = new TextButton(get_text("Train Data"), 153, 50);
    buttons[4] = new TextButton(get_text("Level"), 153, 50);
    buttons[5] = new TextButton(get_text("Train running number"), 153, 50);
    buttons[6] = new TextButton(mode == Mode::SH ? get_text("Exit Shunting") : get_text("Shunting"), 153, 50);
    buttons[7] = new TextButton(get_text("Non-Leading"), 153, 50);
    buttons[8] = new TextButton(get_text("Maintain shunting"), 153, 50);
    buttons[9] = new TextButton(get_text("Radio data"), 153, 50);
    buttons[6]->delayType = true;
    buttons[0]->setPressedAction([this]
    {
        write_command("Start","");
    });
    buttons[1]->setPressedAction([this] 
    {
        write_command("DriverID","");
    });
    buttons[2]->setPressedAction([this] 
    {
        write_command("TrainData","");
    });
    buttons[4]->setPressedAction([this] 
    {
        write_command("Level","");
    });
    buttons[5]->setPressedAction([this] 
    {
        write_command("TrainRunningNumber","");
    });
    buttons[6]->setPressedAction([this]
    {
        write_command("Shunting","");
    });
    buttons[7]->setPressedAction([this]
    { 
        
    });
    buttons[8]->setPressedAction([this]
    { 
        
    });
    buttons[9]->setPressedAction([this]
    { 
        write_command("RadioData","");
    });
    setLayout();
}
void menu_main::setEnabled(bool start, bool driverid, bool traindata, bool level, bool trn, bool sh, bool nl, bool maintainsh, bool radiodata)
{
    buttons[0]->setEnabled(start);
    buttons[1]->setEnabled(driverid);
    buttons[2]->setEnabled(traindata);
    buttons[4]->setEnabled(level);
    buttons[5]->setEnabled(trn);
    buttons[6]->setEnabled(sh);
    buttons[7]->setEnabled(nl);
    buttons[8]->setEnabled(maintainsh);
    buttons[9]->setEnabled(radiodata);
}