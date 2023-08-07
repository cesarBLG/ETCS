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

menu_main::menu_main() : menu(get_text("Main"))
{
    entries[0] = {get_text("Start"), "", []
    {
        write_command("Start","");
    }, false};
    entries[1] = {get_text("Driver ID"), "", []
    {
        write_command("DriverID","");
    }, false};
    entries[2] = {get_text("Train Data"), "", []
    {
        write_command("TrainData","");
    }, false};
    entries[4] = {get_text("Level"), "", []
    {
        write_command("Level","");
    }, false};
    entries[5] = {get_text("Train running number"), "", []
    {
        write_command("TrainRunningNumber","");
    }, false};
    entries[6] = {mode == Mode::SH ? get_text("Exit Shunting") : get_text("Shunting"), "", []
    {
        write_command("Shunting","");
    }, true};
    entries[7] = {get_text("Non-Leading"), "", nullptr, false};
    entries[8] = {get_text("Maintain shunting"), "", nullptr, false};
    entries[9] = {get_text("Radio data"), "", []
    {
        write_command("RadioData","");
    }, false};
    buildMenu();
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