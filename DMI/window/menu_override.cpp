/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "menu_override.h"
#include "../monitor.h"
#include "../tcp/server.h"
menu_override::menu_override() : menu(get_text("Override"))
{
    buttons[0] = new TextButton(get_text("EoA"), 153, 50);
    buttons[0]->setPressedAction([this]
    {
        write_command("Override","");
        //exit(this);
    });
    setLayout();
};
void menu_override::setEnabled(bool eoa)
{
    buttons[0]->enabled = eoa;
}