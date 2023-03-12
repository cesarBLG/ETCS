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
#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include <vector>
#include <string>
#include "../graphics/button.h"
#include "input_data.h"
struct keyboard
{
    std::vector<Button*> keys;
    enum keyboard_type
    {
        NumericKeyboard,
        AlphaNumericKeyboard,
        DedicatedKeyboard
    } type;
};
std::vector<Button*> getNumericKeyboard(input_data *input);
std::vector<Button*> getAlphaNumericKeyboard(input_data *input);
std::vector<Button*> getSingleChoiceKeyboard(std::vector<std::string> posibilities, input_data *input);
std::vector<Button*> getYesNoKeyboard(input_data *input);
std::vector<Button*> getKeyboard(const json &j, input_data *input);
#endif