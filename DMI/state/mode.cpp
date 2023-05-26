/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "mode.h"
#include "../monitor.h"
#include "../graphics/component.h"
#include <string>
using namespace std;
extern Component csg;
Component modeRegion(36,36, displayMode);
static Mode prevmode;
void displayMode()
{
    if(mode==prevmode) return;
    prevmode = mode;
    modeRegion.clear();
    string path = "symbols/Mode/MO_";
    int num;
    switch(mode)
    {
        case Mode::SH:
            num = 1;
            break;
        case Mode::TR:
            num = 4;
            break;
        case Mode::PT:
            num = 6;
            break;
        case Mode::OS:
            num = 7;
            break;
        case Mode::SR:
            num = 9;
            break;
        case Mode::FS:
            num = 11;
            break;
        case Mode::NL:
            num = 12;
            break;
        case Mode::SB:
            num = 13;
            break;
        case Mode::RV:
            num = 14;
            break;
        case Mode::UN:
            num = 16;
            break;
        case Mode::SF:
            num = 18;
            break;
        case Mode::SN:
            num = 19;
            break;
        case Mode::LS:
            num = 21;
            break;
        case Mode::IS:
            num = 23;
            break;
    }
    if(num<10) path+="0";
    path+=to_string(num);
    path+=".bmp";
    modeRegion.addImage(path);
}