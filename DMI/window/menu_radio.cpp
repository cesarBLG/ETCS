/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
menu_radio::menu_radio() : menu(get_text("RBC data"))
{
    buttons[0] = new TextButton(get_text("Contact last RBC"), 153, 50);
    buttons[1] = new TextButton(get_text("Use short number"), 153, 50);
    buttons[2] = new TextButton(get_text("Enter RBC data"), 153, 50);
    buttons[3] = new TextButton(get_text("Radio Network ID"), 153, 50);
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