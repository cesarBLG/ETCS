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
#include "menu_settings.h"
#include "../tcp/server.h"
menu_settings::menu_settings() : menu(get_text("Settings"))
{
    buttons[0] = new IconButton("symbols/Setting/SE_03.bmp", 153, 50);
    buttons[1] = new IconButton("symbols/Setting/SE_02.bmp", 153, 50);
    buttons[2] = new IconButton("symbols/Setting/SE_01.bmp", 153, 50);
    buttons[3] = new TextButton(get_text("System version"), 153, 50);
    buttons[4] = new TextButton(get_text("Set VBC"), 153, 50);
    buttons[5] = new TextButton(get_text("Remove VBC"), 153, 50);
    buttons[0]->setPressedAction([this]
    {
        write_command("Language","");
    });
    buttons[1]->setPressedAction([this] 
    {
        write_command("Volume","");
    });
    buttons[2]->setPressedAction([this] 
    {
        write_command("Brightness","");
    });
    buttons[3]->setPressedAction([this] 
    {
        write_command("SystemVersion","");
    });
    buttons[4]->setPressedAction([this] 
    {
        write_command("SetVBC","");
    });
    buttons[5]->setPressedAction([this] 
    {
        write_command("RemoveVBC","");
    });
    setLayout();
};
void menu_settings::setEnabled(bool language, bool volume, bool brightness, bool version, bool setvbc, bool removevbc)
{
    buttons[0]->setEnabled(language);
    buttons[1]->setEnabled(volume);
    buttons[2]->setEnabled(brightness);
    buttons[3]->setEnabled(version);
    buttons[4]->setEnabled(setvbc);
    buttons[5]->setEnabled(removevbc);
}