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
#include "menu_spec.h"
#include "../tcp/server.h"
menu_spec::menu_spec() : menu("Special")
{
    buttons[0] = new TextButton("Adhesion", 153, 50);
    buttons[1] = new TextButton("SR Speed/Distance", 153, 50);
    buttons[2] = new TextButton("Train integrity", 153, 50);
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