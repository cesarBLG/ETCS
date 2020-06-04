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
#include "../time.h"
#include "../control/control.h"
#include "../tcp/server.h"

menu_main::menu_main() : menu("Main")
{
    buttons[0] = new TextButton("Start", 153, 50);
    buttons[1] = new TextButton("Driver ID", 153, 50);
    buttons[2] = new TextButton("Train Data", 153, 50);
    buttons[3] = new TextButton("Maintain shunting", 153, 50);
    buttons[4] = new TextButton("Level", 153, 50);
    buttons[5] = new TextButton("Train running number", 153, 50);
    buttons[6] = new TextButton(mode == Mode::SH ? "Exit Shunting" : "Shunting", 153, 50);
    buttons[7] = new TextButton("Non-Leading", 153, 50);
    buttons[0]->enabled = (Vest == 0 && mode == Mode::SB && level != Level::Unknown) || (Vest == 0 && mode == Mode::PT && level == Level::N1);
    buttons[2]->enabled = Vest==0;
    buttons[3]->enabled = false;
    buttons[4]->enabled = Vest==0 && (mode == Mode::SB || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN);
    buttons[5]->enabled = (Vest == 0 && mode == Mode::SB && level != Level::Unknown) || (mode == Mode::FS || mode == Mode::LS || mode == Mode::SR || mode == Mode::OS || mode == Mode::NL || mode == Mode::UN || mode == Mode::SN);
    buttons[6]->enabled = Vest == 0;
    buttons[0]->setPressedAction([this]
    {
        write_command("startMission","");
        exit(this);
    });
    buttons[1]->setPressedAction([this] 
    {
        right_menu(new driver_window());
    });
    buttons[2]->setPressedAction([this] 
    {
        right_menu(new fixed_train_data_window());
    });
    buttons[4]->setPressedAction([this] 
    {
        right_menu(new level_window());
    });
    buttons[5]->setPressedAction([this] 
    {
        right_menu(new trn_window());
    });
    buttons[6]->setPressedAction([this]
    {
        write_command("shunting","");
        exit(this);
    });
    buttons[7]->setPressedAction([this]
    { 
        
    });
    setLayout();
}