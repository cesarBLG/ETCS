/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "override.h"
#include "../monitor.h"
void displayOverride();
Component c7(37, 50, displayOverride);
static bool prevOverride;
void displayOverride()
{
    if(prevOverride == ovEOA) return;
    prevOverride = ovEOA;
    c7.clear();
    if(ovEOA) c7.addImage("symbols/Mode/MO_03.bmp");
}