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