/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _MENU_SETTINGS_H
#define _MENU_SETTINGS_H
#include "menu.h"
class menu_settings : public menu
{
    public:
    menu_settings();
    void setEnabled(bool language, bool volume, bool brightness, bool version, bool setvbc, bool removevbc);
};
#endif