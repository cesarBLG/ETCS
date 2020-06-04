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
#include "running_number.h"
#include "../monitor.h"
#include "keyboard.h"
trn_window::trn_window() : input_window("Train running number", 1)
{
    inputs[0] = new trn_input();
    create();
    if (trn_valid && trn != 0)
    {
        inputs[0]->prev_data = inputs[0]->data = to_string(trn);
        inputs[0]->updateText();
    }
}
void trn_window::sendInformation()
{
    trn = stoi(inputs[0]->getData());
    trn_valid = true;
}
trn_input::trn_input()
{
    keys = getNumericKeyboard(this);
}
void trn_input::validate()
{
    if(data.size()>6) return;
    valid = true;
}