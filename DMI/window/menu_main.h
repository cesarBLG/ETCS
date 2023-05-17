/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _MENU_MAIN_H
#define _MENU_MAIN_H
#include "menu.h"
class menu_main : public menu
{
    public:
    menu_main();
    void setEnabled(bool start, bool driverid, bool traindata, bool level, bool trn, bool sh, bool nl, bool maintainsh, bool radiodata);
};
#endif