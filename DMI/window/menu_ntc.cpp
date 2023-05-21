/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "menu_ntc.h"
#include "../tcp/server.h"
menu_ntc::menu_ntc(std::vector<std::string> stms) : menu(get_text("NTC data entry")), stms(stms)
{
    for (int i=0; i<stms.size(); i++)
    {
        buttons[i] = new TextButton(stms[i], 153, 50);
        buttons[i]->setPressedAction([stms,i]
        { 
            write_command("STM",stms[i]);
        });
    }
    buttons[9] = new TextButton(get_text("End of data entry"), 153, 50);
    buttons[9]->setPressedAction([this]
    { 
        write_command("EndDataEntry","");
    });
    setLayout();
}
void menu_ntc::setEnabled(json &enabled)
{
    for (int i=0; i<stms.size(); i++)
    {
        if (enabled.contains(stms[i]) && enabled[stms[i]]) buttons[i]->setEnabled(true);
        else buttons[i]->setEnabled(false);
    }
    buttons[9]->setEnabled(enabled["EndDataEntry"]);
}