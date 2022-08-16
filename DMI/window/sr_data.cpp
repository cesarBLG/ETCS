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
#include "sr_data.h"
#include "keyboard.h"
#include "../tcp/server.h"
#include "../monitor.h"
sr_data_window::sr_data_window() : input_window("SR speed/distance", 2, true)
{
    inputs[0] = new input_data("SR speed (km/h)");
    inputs[1] = new input_data("SR distance (m)");
    for(int i=0; i<2; i++)
    {
        inputs[i]->keys = getNumericKeyboard(inputs[i]);
    }
    create();
}
void sr_data_window::sendInformation()
{
    write_command("setSRspeed", inputs[0]->getData()+","+inputs[1]->getData());
}