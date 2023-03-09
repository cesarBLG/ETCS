/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "menu_main.h"
#include "window.h"
#include "level_window.h"
#include "driver_id.h"
#include "running_number.h"
#include "fixed_train_data.h"
#include <algorithm>
#include "../monitor.h"
#include "../messages/messages.h"
#include "../time_etcs.h"
#include "../control/control.h"
#include "../tcp/server.h"

#include <chrono>
menu_main::menu_main() : menu(gettext("Main"))
{
    buttons[0] = new TextButton(gettext("Start"), 153, 50);
    buttons[1] = new TextButton(gettext("Driver ID"), 153, 50);
    buttons[2] = new TextButton(gettext("Train Data"), 153, 50);
    buttons[4] = new TextButton(gettext("Level"), 153, 50);
    buttons[5] = new TextButton(gettext("Train running number"), 153, 50);
    buttons[6] = new TextButton(mode == Mode::SH ? gettext("Exit Shunting") : gettext("Shunting"), 153, 50);
    buttons[7] = new TextButton(gettext("Non-Leading"), 153, 50);
    buttons[8] = new TextButton(gettext("Maintain shunting"), 153, 50);
    buttons[9] = new TextButton(gettext("Radio data"), 153, 50);
    buttons[6]->delayType = true;
    /*if (wait_rbc)
    {
        for (int i=0; i<8; i++) buttons[i]->enabled = false;
        exit_button.enabled = false;
    }
    else
    {
        buttons[0]->enabled = (Vest == 0 && mode == Mode::SB && level != Level::Unknown) || (Vest == 0 && mode == Mode::PT && level == Level::N1);
        buttons[2]->enabled = Vest==0;
        buttons[3]->enabled = false;
        buttons[4]->enabled = Vest==0 && (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN);
        buttons[5]->enabled = (Vest == 0 && mode == Mode::SB && level != Level::Unknown) || (mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN);
        buttons[6]->enabled = Vest == 0;
    }*/
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