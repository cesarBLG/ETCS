/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "menu_spec.h"
#include "../tcp/server.h"
menu_spec::menu_spec() : menu(get_text("Special"))
{
    entries[0] = {get_text("Adhesion"), "", []
    {
        write_command("Adhesion","");
    }, false};
    entries[1] = {get_text("SR Speed/Distance"), "", []
    {
        write_command("SRspeed","");
    }, false};
    entries[2] = {get_text("Train integrity"), "", []
    {
        write_command("TrainIntegrity","");
    }, true};
#if BASELINE == 4
    entries[3] = {get_text("BMM reaction inhibition"), "", []
    {
        write_command("BTMReaction","");
    }, false};
#endif
    buildMenu();
}
void menu_spec::setEnabled(bool adhesion, bool sr, bool integrity)
{
#if SIMRAIL
    buttons[0]->setEnabled(adhesion);
    buttons[1]->setEnabled(sr);
    buttons[2]->setEnabled(false);
#else
    buttons[0]->setEnabled(adhesion);
    buttons[1]->setEnabled(sr);
    buttons[2]->setEnabled(integrity);
#endif
#if BASELINE == 4
    buttons[3]->setEnabled(false);
#endif
}