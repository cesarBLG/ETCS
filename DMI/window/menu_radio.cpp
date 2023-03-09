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
#include "menu_radio.h"
#include "window.h"
#include <algorithm>
#include "../monitor.h"
#include "../messages/messages.h"
#include "../time_etcs.h"
#include "../control/control.h"
#include "../tcp/server.h"

#include <chrono>
menu_radio::menu_radio() : menu(gettext("Main"))
{
    buttons[0] = new TextButton(gettext("Contact last RBC"), 153, 50);
    buttons[1] = new TextButton(gettext("Use short number"), 153, 50);
    buttons[2] = new TextButton(gettext("Enter RBC data"), 153, 50);
    buttons[3] = new TextButton(gettext("Radio Network ID"), 153, 50);
    buttons[0]->setPressedAction([this]
    {
        write_command("ContactLastRBC","");
    });
    buttons[1]->setPressedAction([this] 
    {
        write_command("UseShortNumber","");
    });
    buttons[2]->setPressedAction([this] 
    {
        write_command("EnterRBCdata","");
    });
    buttons[3]->setPressedAction([this] 
    {
        write_command("RadioNetworkID","");
    });
    setLayout();
}
void menu_radio::setEnabled(bool contactlast, bool shortno, bool enterdata, bool radionetwork)
{
    buttons[0]->setEnabled(contactlast);
    buttons[1]->setEnabled(shortno);
    buttons[2]->setEnabled(enterdata);
    buttons[3]->setEnabled(radionetwork);
}