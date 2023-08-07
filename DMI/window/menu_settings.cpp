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
    entries[0] = {get_text("Language"), "symbols/Setting/SE_03.bmp", []
    {
        write_command("Language","");
    }, false};
    entries[1] = {get_text("Volume"), "symbols/Setting/SE_02.bmp", []
    {
        write_command("Volume","");
    }, false};
    entries[2] = {get_text("Brightness"), "symbols/Setting/SE_01.bmp", []
    {
        write_command("Brightness","");
    }, false};
    entries[3] = {get_text("System version"), "", []
    {
        write_command("SystemVersion","");
    }, false};
    entries[4] = {get_text("Set VBC"), "", []
    {
        write_command("SetVBC","");
    }, false};
    entries[5] = {get_text("Remove VBC"), "", []
    {
        write_command("RemoveVBC","");
    }, false};
    buildMenu();
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