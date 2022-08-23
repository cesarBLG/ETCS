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
#include "rbc_data.h"
#include "../tcp/server.h"
#include "keyboard.h"
rbc_data_window::rbc_data_window(uint32_t id, uint64_t number) : input_window("RBC data", 2, true)
{
    inputs[0] = new input_data("RBC ID", false);
    inputs[0]->keys = getNumericKeyboard(inputs[0]);
    inputs[1] = new input_data("RBC phone number", false);
    inputs[1]->keys = getNumericKeyboard(inputs[1]);
    if (id != 0)
    {
        inputs[0]->data = std::to_string(id);
        inputs[0]->setAccepted(true);
    }
    if (number != 0)
    {
        inputs[1]->data = std::to_string(number);
        inputs[1]->setAccepted(true);
    }
    create();
}
void rbc_data_window::sendInformation()
{
    write_command("setRBCdata", inputs[0]->getData()+","+inputs[1]->getData());
}