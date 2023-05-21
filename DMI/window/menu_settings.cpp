/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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