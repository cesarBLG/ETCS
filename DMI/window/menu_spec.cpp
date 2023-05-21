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
    buttons[0] = new TextButton(get_text("Adhesion"), 153, 50);
    buttons[1] = new TextButton(get_text("SR Speed/Distance"), 153, 50);
    buttons[2] = new TextButton(get_text("Train integrity"), 153, 50);
    buttons[2]->delayType = true;
    buttons[0]->setPressedAction([this]
    {
        write_command("Adhesion","");
    });
    buttons[1]->setPressedAction([this] 
    {
        write_command("SRspeed","");
    });
    buttons[2]->setPressedAction([this] 
    {
        write_command("TrainIntegrity","");
    });
    setLayout();
}
void menu_spec::setEnabled(bool adhesion, bool sr, bool integrity)
{
    buttons[0]->setEnabled(adhesion);
    buttons[1]->setEnabled(sr);
    buttons[2]->setEnabled(integrity);
}