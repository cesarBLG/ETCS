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