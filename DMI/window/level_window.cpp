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
#include "../monitor.h"
#include "../tcp/server.h"
#include "keyboard.h"
level_window::level_window() : input_window("Level", 1)
{
    inputs[0] = new level_input();
    create();
    if (level_valid && level != Level::Unknown)
    {
        string data = "";
        switch(level)
        {
            case Level::N0:
                data = "Level 0";
                break;
            case Level::N1:
                data = "Level 1";
                break;
            case Level::N2:
                data = "Level 2";
                break;
            case Level::N3:
                data = "Level 3";
                break;
        }
        inputs[0]->data = data;
        inputs[0]->prev_data = data;
        inputs[0]->updateText();
    }
}
void level_window::sendInformation()
{
    string data = inputs[0]->getData();
    data = data.substr(6,1);
    level_valid = true;
    write_command("setLevel",data);
}
string str[] = {"Level 0","Level 1", "Level 2", "Level 3", "LZB", "EBICAB"};
level_input::level_input(bool echo) : input_data(echo ? str[(int)level] : "")
{
    keys = getSingleChoiceKeyboard({"Level 1", "Level 2", "Level 3", "Level 0", "LZB", "EBICAB"}, this);
}
void level_input::validate()
{
    if(data.size() < 7) return;
    valid = true;
}

level_validation_window::level_validation_window() : validation_window("Level validation", {new level_input(true)})
{
}
void level_validation_window::sendInformation()
{
    level_valid = true;
}