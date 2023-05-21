/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "flash.h"
#include <cstdio>
#include "drawing.h"
using namespace std;
int flash_state = 0;
Uint32 flash(Uint32 interval, void *param)
{
    flash_state = (flash_state + 1) % 4;
    return interval;
}