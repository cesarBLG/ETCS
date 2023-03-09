/*
 * European Train Control System
 * Copyright (C) 2022  César Benito <cesarbema2009@hotmail.com>
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
#include "set_vbc.h"
#include "../tcp/server.h"
#include "keyboard.h"
set_vbc_window::set_vbc_window() : input_window(gettext("Set VBC"), 1, true)
{
    inputs[0] = new input_data(gettext("VBC code"));
    inputs[0]->keys = getNumericKeyboard(inputs[0]);
    create();
}
void set_vbc_window::sendInformation()
{
    write_command("addVBC", inputs[0]->getData());
}
remove_vbc_window::remove_vbc_window() : input_window(gettext("Remove VBC"), 1, true)
{
    inputs[0] = new input_data(gettext("VBC code"));
    inputs[0]->keys = getNumericKeyboard(inputs[0]);
    create();
}
void remove_vbc_window::sendInformation()
{
    write_command("eraseVBC", inputs[0]->getData());
}