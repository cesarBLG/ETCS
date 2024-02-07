/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../graphics/component.h"
#include "../monitor.h"
#include "../sound/sound.h"

extern bool playSoundOnRadioStatusChange;
void displayRadio();
Component e1(54, 25, displayRadio);
static int prevRadio;
void displayRadio()
{
    if(prevRadio == radioStatus)
        return;

    prevRadio = radioStatus;
    e1.clear();

    if(radioStatus == 1)
        e1.addImage("symbols/Status/ST_04.bmp");
    else if(radioStatus == 2)
        e1.addImage("symbols/Status/ST_03.bmp");

    if (true)
        playSinfo();
}