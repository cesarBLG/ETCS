/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _MENU_RADIO_H
#define _MENU_RADIO_H
#include "menu.h"
class menu_radio : public menu
{
    public:
    menu_radio();
    void setEnabled(bool contactlast, bool shortno, bool enterdata, bool radionetwork);
};
#endif