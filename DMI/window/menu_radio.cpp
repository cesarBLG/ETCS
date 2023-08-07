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

menu_radio::menu_radio() : menu(get_text("RBC data"))
{
    entries[0] = {get_text("Contact last RBC"), "", []
    {
        write_command("ContactLastRBC","");
    }, false};
    entries[1] = {get_text("Use short number"), "", []
    {
        write_command("UseShortNumber","");
    }, false};
    entries[2] = {get_text("Enter RBC data"), "", []
    {
        write_command("EnterRBCdata","");
    }, false};
    entries[3] = {get_text("Radio Network ID"), "", []
    {
        write_command("RadioNetworkID","");
    }, true};
    buildMenu();
}
void menu_radio::setEnabled(bool contactlast, bool shortno, bool enterdata, bool radionetwork)
{
#if SIMRAIL
    buttons[0]->setEnabled(contactlast);
    buttons[1]->setEnabled(false);
    buttons[2]->setEnabled(enterdata);
    buttons[3]->setEnabled(false);
#else
    buttons[0]->setEnabled(contactlast);
    buttons[1]->setEnabled(shortno);
    buttons[2]->setEnabled(enterdata);
    buttons[3]->setEnabled(radionetwork);
#endif
}