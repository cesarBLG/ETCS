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
#include "level_window.h"
#include "../tcp/server.h"
#include "keyboard.h"
level_window::level_window(std::string level, std::vector<std::string> levels) : input_window("Level", 1, false)
{
    inputs[0] = new level_input(level, levels);
    create();
    if (level != "")
    {
        inputs[0]->data = level;
        inputs[0]->prev_data = level;
        inputs[0]->updateText();
    }
}
void level_window::sendInformation()
{
    write_command("setLevel",inputs[0]->getData());
}
level_input::level_input(std::string level, std::vector<std::string> levels, bool echo) : input_data(echo ? "Level" : "")
{
    setData(level);
    keys = getSingleChoiceKeyboard(levels, this);
}
void level_input::validate()
{
    if(data.empty()) return;
    valid = true;
}

level_validation_window::level_validation_window(std::string level) : validation_window("Level validation", {new level_input(level, {""}, true)})
{
}