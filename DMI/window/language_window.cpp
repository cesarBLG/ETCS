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
#include "language_window.h"
#include "../tcp/server.h"
#include "keyboard.h"
language_window::language_window(std::string lang, std::vector<std::string> language) : input_window(gettext("Language"), 1, false)
{
    inputs[0] = new language_input(lang, language);
    create();
    if (lang != "")
    {
        inputs[0]->data = lang;
        inputs[0]->prev_data = lang;
        inputs[0]->updateText();
    }
}
void language_window::sendInformation()
{
    write_command("setLanguage",inputs[0]->getData());
}
language_input::language_input(std::string lang, std::vector<std::string> languages) : input_data("")
{
    setData(lang);
    keys = getSingleChoiceKeyboard(languages, this);
}
void language_input::validate()
{
    if(data.empty()) return;
    valid = true;
}